#include "MediaSession.hh"
#include "FramedSource.hh"

#include "details/_media_session_impl.hpp"
#include "details/media_framers/_media_framer_base.hpp"
#include "sample_attributes.hpp"

#ifdef min
#undef min
#endif

using namespace std::chrono;
using namespace uvxx::rtsp;
using namespace uvxx::rtsp::sample_attributes;
using namespace uvxx::rtsp::details;
using namespace uvxx::rtsp::details::media_framers;

static const size_t DEFAULT_READ_BUFFER_SIZE = 320 * 1024;
static const size_t MAX_READ_BUFFER_SIZE     = 2   * 1024 * 1024;


_media_framer_base::_media_framer_base(const media_subsession& subsession) :
    _subsession(std::move(subsession)),
    _last_presentation_time(0),
    _current_presentation_time(0),
    _was_synced(false),
    _use_rtp_marker_for_pts(false)
{
    auto live_subsession = subsession.__media_subsession->live_media_subsession();
   
    FramedSource* framed_source = live_subsession->readSource();

    if (!framed_source)
    {
        return;
    }

    if (live_subsession->rtcpInstance())
    {
        sample_major_type media_major = sample_major_type::unknown;

        auto medium_name = subsession.medium_name();

        if (!strcmp(medium_name.c_str(), "video"))
        {
            media_major = sample_major_type::video;
        }
        else if (!strcmp(medium_name.c_str(), "audio"))
        {
            media_major = sample_major_type::audio;
        }

        if(media_major == sample_major_type::video)
        {
            _sample.capacity_set(DEFAULT_READ_BUFFER_SIZE);
        }
        else
        {
            _sample.capacity_set(DEFAULT_READ_BUFFER_SIZE / 8);
        }

        _sample.stream_number_set(subsession.stream_number());

        _sample.codec_name_set(subsession.codec_name());

        _sample.attribute_set(ATTRIBUTE_SAMPLE_MAJOR_TYPE, media_major);

        /* set a 'BYE' handler for this subsession's RTCP instance: */
        live_subsession->rtcpInstance()->setByeHandler(on_rtcp_bye, this);
    }
}

_media_framer_base::~_media_framer_base()
{
    auto live_subsession = _subsession.__media_subsession->live_media_subsession();

    FramedSource* subsessionSource = live_subsession->readSource();

    if (subsessionSource)
    {
        subsessionSource->stopGettingFrames();
    }

    live_subsession->rtcpInstance()->setByeHandler(nullptr, nullptr);
}

void _media_framer_base::begin_reading()
{
    continue_reading();
}

void _media_framer_base::on_sample_set(read_sample_delegate callback)
{
    _sample_callback = std::move(callback);
}

void _media_framer_base::on_stream_closed_set(stream_closed_delegate callback)
{
    _stream_closed_delegate = std::move(callback);
}

void _media_framer_base::continue_reading()
{
    auto live_subsession = _subsession.__media_subsession->live_media_subsession();

    FramedSource* framed_source = live_subsession->readSource();

    if (!framed_source)
    {
        return;
    }

    if (framed_source->isCurrentlyAwaitingData())
    {
        return;
    }

    framed_source->getNextFrame(const_cast<unsigned char*>(_sample.data()),
                                _sample.capacity(),
                                on_after_getting_frame,
                                this,
                                on_rtcp_bye,
                                this);
}

void _media_framer_base::use_rtp_marker_for_pts_set(bool use_rtp_marker)
{
    _use_rtp_marker_for_pts = use_rtp_marker;
}

_qos_stats& _media_framer_base::qos_stats_get()
{
    return __qos_stats;
}

int _media_framer_base::stream_number()
{
    return _subsession.stream_number();
}

media_sample _media_framer_base::working_sample()
{
    return _sample;
}

void _media_framer_base::sample_receieved(bool /*packet_marker_bit*/)
{
    do_sample_callback();
}

void _media_framer_base::do_sample_callback()
{
    if (_sample_callback)
    {
        _sample_callback(_sample);
    }
}

void _media_framer_base::on_after_getting_frame(void* client_data, unsigned packet_data_size, unsigned truncated_bytes, struct timeval presentation_time, unsigned duration_in_microseconds)
{
    auto base = static_cast<_media_framer_base*>(client_data);

    base->on_after_getting_frame(packet_data_size, truncated_bytes, presentation_time, duration_in_microseconds);
}

void _media_framer_base::on_after_getting_frame(unsigned packet_data_size, unsigned truncated_bytes, struct timeval presentation_time, unsigned /*duration_in_microseconds*/)
{
    static uint64_t ONE_MILLION = 1000000ull;

    auto live_subsession = _subsession.__media_subsession->live_media_subsession();

    FramedSource* framed_source = live_subsession->readSource();

    framed_source->stopGettingFrames();

    if (truncated_bytes)
    {
        adjust_buffer_for_trucated_bytes(truncated_bytes, _sample);
    }

    bool marker_bit = true;

    bool is_synced = true;

    if (framed_source->isRTPSource())
    {
        auto rtp_source = static_cast<RTPSource*>(framed_source);

        RTPReceptionStatsDB::Iterator statsIter(rtp_source->receptionStatsDB());

        /* assumes only one ssrc - apparently the usual case */
        auto stats = statsIter.next(True);

        if (stats) 
        {
            __qos_stats.record_stats(*stats);
        }

        is_synced = rtp_source->hasBeenSynchronizedUsingRTCP();

        marker_bit = rtp_source->curPacketMarkerBit();
    }

    microseconds reported_micro_seconds(ONE_MILLION * presentation_time.tv_sec + presentation_time.tv_usec);
    
    if (_last_presentation_time == microseconds::zero() || 
       (is_synced && !_was_synced))
    {
        _last_presentation_time = reported_micro_seconds;

        _presentation_time_base = reported_micro_seconds;
    }

    _was_synced = is_synced;
    
    if (marker_bit || !_use_rtp_marker_for_pts)
    {
        auto sample_duration = microseconds(std::abs(reported_micro_seconds.count() - _last_presentation_time.count()));

        _current_presentation_time += sample_duration;

        _last_presentation_time = reported_micro_seconds;
    }

    _sample.presentation_time_set(_current_presentation_time);

    _sample.is_truncated_set(truncated_bytes > 0);

    _sample.size_set(packet_data_size);
    
    sample_receieved(marker_bit);
}

void _media_framer_base::on_rtcp_bye(void* client_data)
{
    assert(client_data);
    
    auto framer = static_cast<_media_framer_base*>(client_data);
    
    if (framer->_stream_closed_delegate)
    {
        framer->_stream_closed_delegate(framer->_subsession.stream_number());
    }
}

void _media_framer_base::adjust_buffer_for_trucated_bytes(unsigned truncated_amount, media_sample& sample)
{
    if (truncated_amount == 0)
    {
        return;
    }

    size_t current_size = sample.capacity();

    if (current_size == MAX_READ_BUFFER_SIZE)
    {
        return;
    }

    size_t new_size = current_size + (truncated_amount * 2);

    new_size = std::min(MAX_READ_BUFFER_SIZE, new_size);

    if (new_size == current_size)
    {
        return;
    }

    printf("resizing buffer to %zu\n", new_size);

    sample.capacity_set(new_size);
}

#include "MediaSession.hh"
#include "FramedSource.hh"
#include "details/_media_session_impl.hpp"
#include "details/media_framers/_media_framer_base.hpp"
#include "sample_attributes.hpp"

using namespace std::chrono;
using namespace uvxx::rtsp;
using namespace uvxx::rtsp::sample_attributes;
using namespace uvxx::rtsp::details;
using namespace uvxx::rtsp::details::media_framers;

_media_framer_base::_media_framer_base(const media_subsession& subsession) :
    _subsession(std::move(subsession)),
    _lastPresentationTime(0),
    _currentPresentationTime(0),
    _was_synced(false)
{
    auto live_subsession = subsession.__media_subsession->live_media_subsession();
   
    FramedSource* framed_source = live_subsession->readSource();

    if (!framed_source)
    {
        return;
    }

    if (live_subsession->rtcpInstance())
    {
        media_sample sample;

        _sample.capacity_set(1024 * 200);

        _sample.stream_number_set(subsession.stream_number());

        _sample.codec_name_set(live_subsession->codecName());

        sample_major_type media_major = sample_major_type::unknown;

        if (!strcmp(live_subsession->mediumName(), "video"))
        {
            media_major = sample_major_type::video;
        }
        else if (!strcmp(live_subsession->mediumName(), "audio"))
        {
            media_major = sample_major_type::audio;
        }

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

    live_subsession->rtcpInstance()->setByeHandler(on_rtcp_bye, nullptr);
}

void _media_framer_base::begin_reading()
{
    continue_reading();
}

void _media_framer_base::on_sample_set(read_sample_delegate callback)
{
    _sample_callback = std::move(callback);
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
                                nullptr,
                                nullptr);
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

    if (truncated_bytes)
    {
        adjust_buffer_for_trucated_bytes(truncated_bytes, _sample);
    }

    bool marker_bit = true;

    bool is_synced = true;

    if (framed_source->isRTPSource())
    {
        auto rtp_source = static_cast<RTPSource*>(framed_source);

        is_synced = rtp_source->hasBeenSynchronizedUsingRTCP();

        marker_bit = rtp_source->curPacketMarkerBit();
    }

    microseconds reported_micro_seconds((ONE_MILLION * presentation_time.tv_sec) + presentation_time.tv_usec);
    
    if (_lastPresentationTime == microseconds::zero() || 
       (is_synced && !_was_synced))
    {
        _lastPresentationTime = reported_micro_seconds;
    }

    _was_synced = is_synced;
    
    auto sample_duration = microseconds(std::abs(reported_micro_seconds.count() - _lastPresentationTime.count()));
   
    _currentPresentationTime += sample_duration;

    _lastPresentationTime = reported_micro_seconds;

    _sample.presentation_time_set(_currentPresentationTime);

    _sample.is_truncated_set(truncated_bytes > 0);

    _sample.size_set(packet_data_size);
    
    sample_receieved(marker_bit);
}

void _media_framer_base::on_rtcp_bye(void* /*client_data*/)
{

}
#ifdef min
#undef min
#endif
void _media_framer_base::adjust_buffer_for_trucated_bytes(unsigned truncated_amount, const media_sample& sample)
{
    static const size_t MAX_BUFFER_SIZE = 2 * 1024 * 1024;

    if (truncated_amount == 0)
    {
        return;
    }

    size_t current_size = sample.capacity();

    if (current_size == MAX_BUFFER_SIZE)
    {
        return;
    }

    size_t new_size = current_size + (truncated_amount * 2);

    new_size = std::min(MAX_BUFFER_SIZE, new_size);

    if (new_size == current_size)
    {
        return;
    }

    printf("resizing buffer to %u\n", new_size);

    sample.capacity_set(new_size);
}

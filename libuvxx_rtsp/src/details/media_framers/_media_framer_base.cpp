#include "details/media_framers/_media_framer_base.h"

using namespace std::chrono;
using namespace uvxx::rtsp;
using namespace uvxx::rtsp::details;
using namespace uvxx::rtsp::details::media_framers;

_media_framer_base::_media_framer_base(const media_subsession& subsession, int stream_number) :
    _subsession(std::move(subsession)),
    _stream_number(stream_number),
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

        //_sample.stream_number_set(stream_number);

        _sample.codec_name_set(live_subsession->codecName());

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

void _media_framer_base::begin_reading(std::function<bool(const media_sample&)> callback)
{
    _sample_callback = std::move(callback);

    if (!_sample_callback)
    {
        return;
    }

    continue_reading();
}


void uvxx::rtsp::details::media_framers::_media_framer_base::continue_reading()
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

    framed_source->getNextFrame((unsigned char*) _sample.data(),
                                _sample.capacity(),
                                on_after_getting_frame,
                                this,
                                nullptr,
                                nullptr);
}

int uvxx::rtsp::details::media_framers::_media_framer_base::stream_number()
{
    return _stream_number;
}

uvxx::rtsp::media_sample uvxx::rtsp::details::media_framers::_media_framer_base::sample()
{
    return _sample;
}

void uvxx::rtsp::details::media_framers::_media_framer_base::sample_receieved()
{
    do_sample_callback();

    continue_reading();
}

void uvxx::rtsp::details::media_framers::_media_framer_base::do_sample_callback()
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

void _media_framer_base::on_after_getting_frame(unsigned packet_data_size, unsigned truncated_bytes, struct timeval presentation_time, unsigned duration_in_microseconds)
{
    static uint64_t ONE_MILLION = 1000000ull;

    auto live_subsession = _subsession.__media_subsession->live_media_subsession();

    FramedSource* framed_source = live_subsession->readSource();

    if (truncated_bytes)
    {
        adjust_buffer_for_trucated_bytes(truncated_bytes, _sample);
    }

    bool is_complete_sample = true;

    bool is_synced = true;

    if (framed_source->isRTPSource())
    {
        auto rtp_source = static_cast<RTPSource*>(framed_source);

        is_synced = rtp_source->hasBeenSynchronizedUsingRTCP();

        is_complete_sample = rtp_source->curPacketMarkerBit();
    }

    microseconds reported_micro_seconds((ONE_MILLION * presentation_time.tv_sec) + presentation_time.tv_usec);
    
    if (_lastPresentationTime == microseconds::zero() || 
        is_synced && !_was_synced)
    {
        _lastPresentationTime = reported_micro_seconds;
    }

    _was_synced = is_synced;
    
    auto sample_duration = reported_micro_seconds - _lastPresentationTime;
   
    _currentPresentationTime += sample_duration;

    _lastPresentationTime = reported_micro_seconds;

    _sample.presentation_time_set(_currentPresentationTime);

    _sample.is_truncated_set(truncated_bytes > 0);

    _sample.size_set(packet_data_size);

    sample_receieved();
}

void _media_framer_base::on_rtcp_bye(void* client_data)
{

}

void _media_framer_base::adjust_buffer_for_trucated_bytes(unsigned truncated_amount, const uvxx::rtsp::media_sample& sample)
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

    new_size = max(MAX_BUFFER_SIZE, new_size);

    if (new_size == current_size)
    {
        return;
    }

    printf("resizing buffer to %u\n", new_size);

    sample.capacity_set(new_size);
}

#include "details/_streaming_media_session_impl.hpp"
#include "details/_media_session_impl.hpp"
#include "MediaSession.hh"

using namespace uvxx::pplx;
using namespace uvxx::rtsp;
using namespace uvxx::rtsp::details;

struct _streaming_media_io_state
{
    _streaming_media_io_state() = default;

    _streaming_media_io_state(const _streaming_media_io_state&) = delete;

    _streaming_media_io_state& operator=(const _streaming_media_io_state&) = delete;
   
    int stream_number;

    MediaSubsession* live_subsession;

    _streaming_media_session_impl* _streaming_media_session;

    media_sample sample;
};

_streaming_media_session_impl::_streaming_media_session_impl(const media_session& session, 
                                                             std::vector<media_subsession> subsessions) :
    _session(session),
    _subsessions(std::move(subsessions))
{
    _buffer.resize(1024 * 100);

    int stream_number = 0;

    for (auto& subsession : _subsessions)
    {
        auto live_subsession = subsession.__media_subsession->live_media_subsession();

        live_subsession->miscPtr = nullptr;

        FramedSource* framed_source = live_subsession->readSource();

        if(!framed_source)
        {
            continue;
        }

        /* set a 'BYE' handler for this subsession's RTCP instance: */
        if (live_subsession->rtcpInstance()) 
        {
            media_sample sample;

            sample.__media_sample_impl->stream_number_set(stream_number);

            sample.__media_sample_impl->codec_name_set(live_subsession->codecName());

            assert(!live_subsession->miscPtr);

            live_subsession->miscPtr = new _streaming_media_io_state{stream_number, live_subsession, this, sample};

            live_subsession->rtcpInstance()->setByeHandler(on_rtcp_bye, this);
        }

        stream_number++;
    }
}

void _streaming_media_session_impl::on_rtcp_bye(void* client_data)
{

}

_streaming_media_session_impl::~_streaming_media_session_impl()
{
    for (auto& subsession : _subsessions)
    {
        auto live_subsession = subsession.__media_subsession->live_media_subsession();

        if (live_subsession->rtcpInstance()) 
        {
            if (live_subsession->miscPtr)
            {
                auto state = static_cast<_streaming_media_io_state*>(live_subsession->miscPtr);

                delete state;

                live_subsession->miscPtr = nullptr;
            }

            FramedSource* subsessionSource = live_subsession->readSource();

            if (subsessionSource)
            {
                subsessionSource->stopGettingFrames();
            }

            live_subsession->rtcpInstance()->setByeHandler(on_rtcp_bye, nullptr);
        }
    }
}

void uvxx::rtsp::details::_streaming_media_session_impl::on_frame_callback_set(std::function<bool(const media_sample&)> callback)
{
    _on_frame_callback = std::move(callback);

    continue_reading();
}

void uvxx::rtsp::details::_streaming_media_session_impl::continue_reading()
{
    for (auto& subsession : _subsessions)
    {
        auto live_subsession = subsession.__media_subsession->live_media_subsession();

        FramedSource* framed_source = live_subsession->readSource();

        if (!framed_source)
        {
            continue;
        }

        if (framed_source->isCurrentlyAwaitingData())
        {
            continue;
        }

        framed_source->getNextFrame(_buffer.data(), 
                                    _buffer.size(), 
                                    on_after_getting_frame, 
                                    live_subsession->miscPtr, 
                                    nullptr, 
                                    nullptr);
    }
}

void _streaming_media_session_impl::adjust_buffer_for_trucated_bytes(unsigned truncated_amount)
{
    static const size_t MAX_BUFFER_SIZE = 2 * 1024 * 1024;

    if (truncated_amount == 0)
    {
        return;
    }

    size_t current_size = _buffer.size();

    size_t new_size = current_size + (truncated_amount * 2);

    new_size = max(MAX_BUFFER_SIZE, new_size);

    if (new_size == current_size)
    {
        return;
    }

    printf("resizing buffer to %u\n", new_size);

    _buffer.resize(new_size);
}

void _streaming_media_session_impl::on_after_getting_frame(void* client_data, 
                                                           unsigned packet_data_size, 
                                                           unsigned truncated_bytes, 
                                                           struct timeval presentation_time, 
                                                           unsigned duration_in_microseconds)
{
    static uint64_t ONE_MILLION = 1000000ull;

    auto io_state = static_cast<_streaming_media_io_state*>(client_data);

    if (truncated_bytes)
    {
        io_state->_streaming_media_session->adjust_buffer_for_trucated_bytes(truncated_bytes);
    }

    FramedSource* framed_source = io_state->live_subsession->readSource();

    auto& sample = io_state->sample;

    auto& sample_impl = sample.__media_sample_impl;

    std::chrono::microseconds micro_seconds((ONE_MILLION * presentation_time.tv_sec) + 
                                            presentation_time.tv_usec);

    sample_impl->presentation_time_set(micro_seconds);

    sample_impl->is_truncated_set(truncated_bytes > 0);

    sample_impl->size_set(packet_data_size);

    bool is_complete_sample = true;

    bool is_synced = true;

    if (framed_source->isRTPSource())
    {
        auto rtp_source = static_cast<RTPSource*>(framed_source);

        is_synced = rtp_source->hasBeenSynchronizedUsingRTCP();

        is_complete_sample = rtp_source->curPacketMarkerBit();

        sample_impl->is_complete_sample_set(is_complete_sample);
    }
    else
    {
        sample_impl->is_complete_sample_set(true);
    }

    if (!is_synced)
    {
        io_state->_streaming_media_session->continue_reading();

        return;
    }

    bool continue_reading = io_state->_streaming_media_session->_on_frame_callback(sample);

    if (continue_reading)
    {
        io_state->_streaming_media_session->continue_reading();
    }
}

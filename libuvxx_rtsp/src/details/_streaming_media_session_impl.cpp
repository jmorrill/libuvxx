#include "details/_streaming_media_session_impl.hpp"
#include "details/_media_session_impl.hpp"
#include "MediaSession.hh"

using namespace uvxx::pplx;
using namespace uvxx::rtsp;
using namespace uvxx::rtsp::details;

struct _streaming_media_io_state
{
    MediaSubsession* live_subsession;

    _streaming_media_session_impl* _streaming_media_session;
};

_streaming_media_session_impl::_streaming_media_session_impl(const media_session& session, std::vector<media_subsession> subsessions) :
    _session(session),
    _subsessions(std::move(subsessions))
{
    _buffer.resize(50000);

    for (auto& subsession : _subsessions)
    {
        auto live_subsession = subsession.__media_subsession->live_media_subsession();

        live_subsession->miscPtr = nullptr;

        FramedSource* framed_source = live_subsession->readSource();

        if(!framed_source)
        {
            continue;
        }

        framed_source->stopGettingFrames();

        /* set a 'BYE' handler for this subsession's RTCP instance: */
        if (live_subsession->rtcpInstance()) 
        {
            live_subsession->miscPtr = new _streaming_media_io_state{ live_subsession, this};

            live_subsession->rtcpInstance()->setByeHandler(on_rtcp_bye, this);
        }
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

void uvxx::rtsp::details::_streaming_media_session_impl::on_frame_callback_set(std::function<bool()> callback)
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

        if (framed_source == nullptr)
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
                                    live_subsession->miscPtr);
    }
}

void _streaming_media_session_impl::on_after_getting_frame(void* client_data, unsigned packet_data_size, unsigned truncated_bytes, struct timeval presentation_time, unsigned duration_in_microseconds)
{
    auto io_state = static_cast<_streaming_media_io_state*>(client_data);

    bool continue_reading = io_state->_streaming_media_session->_on_frame_callback();

    io_state->live_subsession->codecName();
    printf("%s\t size: %d\ttruncated: %d\ttime: %u\n", io_state->live_subsession->codecName(), packet_data_size, truncated_bytes, presentation_time.tv_sec);
    if (continue_reading)
    {
        io_state->_streaming_media_session->continue_reading();
    }
}

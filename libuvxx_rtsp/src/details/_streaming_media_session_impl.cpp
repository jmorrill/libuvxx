#include "details/_streaming_media_session_impl.hpp"
#include "details/_media_session_impl.hpp"
#include "MediaSession.hh"

using namespace uvxx::pplx;
using namespace uvxx::rtsp;
using namespace uvxx::rtsp::details;

class _live_media_session_sink : Medium
{
public:
    _live_media_session_sink(UsageEnvironment& live_environment) : Medium(live_environment)
    {

    }
};

struct _streaming_media_io_state
{
    MediaSubsession* live_subsession;

    _streaming_media_session_impl* _streaming_media_session;
};

_streaming_media_session_impl::_streaming_media_session_impl(const std::vector<media_subsession>& subsessions)
{
    _subsessions = subsessions;

    for (auto& subsession : _subsessions)
    {
        auto live_subsession = subsession.__media_subsession->live_media_subsession_get();

        FramedSource* subsessionSource = live_subsession->readSource();

        if(!subsessionSource)
        {
            continue;
        }

        /* set a 'BYE' handler for this subsession's RTCP instance: */
        if (live_subsession->rtcpInstance() != nullptr) 
        {
            live_subsession->miscPtr = new _streaming_media_io_state{ live_subsession, this};

            live_subsession->rtcpInstance()->setByeHandler(on_rtcp_bye, this);
        }
    }
}

void uvxx::rtsp::details::_streaming_media_session_impl::on_rtcp_bye(void* client_data)
{

}

uvxx::rtsp::details::_streaming_media_session_impl::~_streaming_media_session_impl()
{
    for (auto& subsession : _subsessions)
    {
        auto live_subsession = subsession.__media_subsession->live_media_subsession_get();

        if (live_subsession->rtcpInstance() != nullptr) 
        {
            if (live_subsession->miscPtr)
            {
                delete live_subsession->miscPtr;
                live_subsession->miscPtr = nullptr;
            }

            live_subsession->rtcpInstance()->setByeHandler(on_rtcp_bye, nullptr);
        }
    }
}

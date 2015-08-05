#include "BasicUsageEnvironment.hh"
#include "ServerMediaSession.hh"

#include "event_dispatcher_frame.hpp"
#include "details/_rtsp_server_impl.hpp"
#include "details/_live_rtsp_server.hpp"
#include "details/_uvxx_task_scheduler.hpp"
#include "details/_server_media_session_impl.hpp"
#include "details/_live_server_media_session.hpp"
#include "details/_h264_media_subsession.hpp"
#include "server_media_session.hpp"

using namespace uvxx::rtsp::details;

_rtsp_server_impl::_rtsp_server_impl(uint16_t port) :
    _port(0)
{
    _task_scheduler = _uvxx_task_scheduler::createNew();

    _usage_environment = _usage_environment_ptr(BasicUsageEnvironment::createNew(*_task_scheduler),
    /* deleter*/
    [](UsageEnvironment* env)
    {
        auto& task_scheduler = env->taskScheduler();

        delete &task_scheduler;

        if (!env->reclaim())
        {
            assert(false);
        }
    });

    _live_server = _live_rtsp_server_ptr(new _live_rtsp_server(_usage_environment, port),
    /* deleter */
    [](_live_rtsp_server* client)
    {
        Medium::close(client);
    });

    _live_server->set_on_lookup_media_session(std::bind(&_rtsp_server_impl::on_live_media_session_lookup, this, std::placeholders::_1));
}

uint16_t _rtsp_server_impl::port()
{
    return _port;
}

ServerMediaSession* _rtsp_server_impl::on_live_media_session_lookup(const std::string& stream_name)
{
    auto dispatcher = event_dispatcher_object::dispatcher();

    uvxx::rtsp::server_media_session _server_media_session;

    event_dispatcher_frame frame;

    dispatcher.begin_invoke([=]() mutable
    {
        frame.continue_set(false);
    });
    
    dispatcher.push_frame(frame);
    
    auto session = _server_media_session.__server_media_session_impl->__live_server_media_session.get();

    session->addSubsession(new _h264_media_subsession(_server_media_session.__server_media_session_impl->_usage_environment));

    session->is_externally_owned_set(true);

    return session;
}
#include "BasicUsageEnvironment.hh"
#include "ServerMediaSession.hh"

#include "event_dispatcher_frame.hpp"

#include "server_media_session.hpp"
#include "rtsp_server.hpp"

#include "details/_rtsp_server_impl.hpp"
#include "details/_live_rtsp_server.hpp"
#include "details/_uvxx_task_scheduler.hpp"
#include "details/_server_media_session_impl.hpp"
#include "details/_live_server_media_session.hpp"
#include "details/_h264_media_subsession.hpp"

using namespace uvxx::rtsp::details;

_rtsp_server_impl::_rtsp_server_impl(uint16_t port) :
    _port(0)
{
    _live_server = _live_rtsp_server_ptr(new _live_rtsp_server(port),
    [](_live_rtsp_server* server)
    {
        Medium::close(server);
    });

    _live_server->set_on_lookup_media_session(std::bind(&_rtsp_server_impl::on_live_media_session_lookup, this, std::placeholders::_1));
}

uint16_t _rtsp_server_impl::port()
{
    return _port;
}

void _rtsp_server_impl::on_session_request_set(uvxx::rtsp::on_session_request_delegate callback)
{
    _on_session_request_delegate = callback;
}

ServerMediaSession* _rtsp_server_impl::on_live_media_session_lookup(const std::string& stream_name)
{
    if(!_on_session_request_delegate)
    {
        return nullptr;
    }

    auto dispatcher = event_dispatcher_object::dispatcher();


    event_dispatcher_frame frame;

    server_media_session server_session;

    auto t = _on_session_request_delegate(stream_name).
    then([=, &server_session, &frame](uvxx::pplx::task<server_media_session> get_session_task) mutable
    {
        try
        {
            server_session = get_session_task.get();
        }
        catch (const std::exception&)
        {
            printf("err");
        }

        frame.continue_set(false);
    });
    
    /* This callback requires to be syncronous and expects a result after
       returning.  Here we fake async by entering a dispatcher frame */
    dispatcher.push_frame(frame);
    
    auto session = server_session.__server_media_session_impl->__live_server_media_session.get();

    session->is_externally_owned_set(true);

    return session;
}
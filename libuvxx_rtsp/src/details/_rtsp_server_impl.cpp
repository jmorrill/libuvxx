#include "BasicUsageEnvironment.hh"
#include "ServerMediaSession.hh"

#include "event_dispatcher_frame.hpp"

#include "server_media_session.hpp"
#include "rtsp_server.hpp"

#include "details/_rtsp_server_impl.hpp"
#include "details/_live_rtsp_server.hpp"
#include "details/_server_media_session_impl.hpp"
#include "details/_live_server_media_session.hpp"

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

uvxx::pplx::task<_live_server_media_session*> _rtsp_server_impl::on_live_media_session_lookup(const std::string& stream_name)
{
    if(!_on_session_request_delegate)
    {
        return pplx::task_from_result<_live_server_media_session*>(nullptr);
    }

    auto dispatcher = event_dispatcher_object::dispatcher();


    return _on_session_request_delegate(stream_name).then([=](uvxx::pplx::task<server_media_session> get_session_task) mutable
    {
        try
        {
            server_media_session server_session = std::move(get_session_task.get());

            auto session = server_session.__server_media_session_impl->__live_server_media_session.get();

            if(session)
            {
                session->is_externally_owned_set(true);
                
                return static_cast<_live_server_media_session*>(session);
            }
        }
        catch (const std::exception&)
        {
        }

        return static_cast<_live_server_media_session*>(nullptr);
    });
}
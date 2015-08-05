#pragma once
#include <memory>

class UsageEnvironment;

class ServerMediaSession;

namespace uvxx { namespace rtsp { namespace details 
{
    class _live_rtsp_server;

    using _live_rtsp_server_ptr = std::shared_ptr<_live_rtsp_server>;

    using _usage_environment_ptr = std::shared_ptr<UsageEnvironment>;
   
    class _rtsp_server_impl : public event_dispatcher_object
    {
    public:
        _rtsp_server_impl(uint16_t port);

    public:
        uint16_t port();

    private:
        ServerMediaSession* on_live_media_session_lookup(const std::string& stream_name);

    private:
        _usage_environment_ptr _usage_environment;

        _live_rtsp_server_ptr _live_server;

        uint16_t _port;
    };
}}}
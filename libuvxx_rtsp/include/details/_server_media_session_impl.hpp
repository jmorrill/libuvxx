#pragma once
#include "event_dispatcher_object.hpp"

class UsageEnvironment;

class ServerMediaSession;

namespace uvxx { namespace rtsp { namespace details 
{
    class _rtsp_server_impl;

    class _uvxx_task_scheduler;

    class _live_server_media_session;

    using _live_server_media_session_ptr = std::shared_ptr<_live_server_media_session>;

    using _usage_environment_ptr = std::shared_ptr<UsageEnvironment>;

    class _server_media_session_impl : public event_dispatcher_object
    {
    public:
        _server_media_session_impl();

    private:
        _uvxx_task_scheduler* _task_scheduler;

        _usage_environment_ptr _usage_environment;

        _live_server_media_session_ptr __live_server_media_session;

        friend _rtsp_server_impl;
    };
}}}
#pragma once
#include <memory>
#include "ServerMediaSession.hh"

class UsageEnvironment;

namespace uvxx { namespace rtsp { namespace details 
{
    class _rtsp_server_impl;

    using _usage_environment_ptr = std::shared_ptr<UsageEnvironment>;

    class _live_server_media_session : public ServerMediaSession
    {
    public:
        _live_server_media_session(const _usage_environment_ptr& environment);

        _live_server_media_session(const _live_server_media_session&) = delete;

        _live_server_media_session& operator=(const _live_server_media_session) = delete;

        virtual ~_live_server_media_session()
        {
        }

    private:
        _usage_environment_ptr _usage_environment;

        friend _rtsp_server_impl;
    };
}}}
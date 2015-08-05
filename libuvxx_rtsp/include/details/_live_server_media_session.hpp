#pragma once
#include <memory>
#include <functional>
#include "ServerMediaSession.hh"

class UsageEnvironment;

namespace uvxx { namespace rtsp { namespace details 
{
    class _rtsp_server_impl;

    using _usage_environment_ptr = std::shared_ptr<UsageEnvironment>;

    using _live_session_closed_delegate = std::function<void()>;

    class _live_server_media_session : public ServerMediaSession
    {
    public:
        _live_server_media_session(const _usage_environment_ptr& environment);

        _live_server_media_session(const _live_server_media_session&) = delete;

        _live_server_media_session& operator=(const _live_server_media_session) = delete;

        virtual ~_live_server_media_session();

        bool is_externally_owned() const;

        void is_externally_owned_set(bool is_externally_owned);

    public:
        void on_session_closed(_live_session_closed_delegate callback);

    private:
        _usage_environment_ptr _usage_environment;

        _live_session_closed_delegate __live_session_closed_delegate;

        bool _is_externally_owned;

        friend _rtsp_server_impl;
    };
}}}
#pragma once
#include <functional>
#include "ServerMediaSession.hh"
#include "_live_environment_base.hpp"
#include <pplx/pplxtasks.h>

namespace uvxx { namespace rtsp { namespace details 
{
    class _rtsp_server_impl;

    using _live_session_closed_delegate = std::function<void()>;

    class _live_server_media_session : public ServerMediaSession, protected _live_environment_base
    {
    public:
        _live_server_media_session();

        _live_server_media_session(const _live_server_media_session&) = delete;

        _live_server_media_session& operator=(const _live_server_media_session) = delete;

        virtual ~_live_server_media_session();

        bool is_externally_owned() const;

        void is_externally_owned_set(bool is_externally_owned);

    public:
        void on_session_closed(_live_session_closed_delegate callback);

        pplx::task<void> preload_sdp_data();
    private:
        _live_session_closed_delegate __live_session_closed_delegate;

        bool _is_externally_owned;

        friend _rtsp_server_impl;
    };
}}}
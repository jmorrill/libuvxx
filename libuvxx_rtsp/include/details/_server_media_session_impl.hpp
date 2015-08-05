#pragma once
#include "event_dispatcher_object.hpp"

namespace uvxx { namespace rtsp { namespace details 
{
    class _rtsp_server_impl;

    class _live_server_media_session;

    using _live_server_media_session_ptr = std::shared_ptr<_live_server_media_session>;

    class _server_media_session_impl : public event_dispatcher_object
    {
    public:
        _server_media_session_impl();

        virtual ~_server_media_session_impl();

    private:
        void on_session_closed();

    private:
        _live_server_media_session_ptr __live_server_media_session;

        friend _rtsp_server_impl;
    };
}}}
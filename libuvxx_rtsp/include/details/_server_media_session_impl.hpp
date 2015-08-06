#pragma once
#include "event_dispatcher_object.hpp"
#include "media_descriptor.hpp"

namespace uvxx { namespace rtsp {
    class media_sample;
    class media_descriptor;
}}

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

        _server_media_session_impl(const _server_media_session_impl& rhs) = default;

        _server_media_session_impl& operator=(const _server_media_session_impl&) = default;

        _server_media_session_impl(_server_media_session_impl&& rhs);

        _server_media_session_impl& operator=(_server_media_session_impl&& rhs);

    public:
        void set_media_descriptor(const media_descriptor& descriptor);

        void deliver_sample(int stream_id, const media_sample& sample);

    private:
        void on_session_closed();

        void configure_session();

    private:
        _live_server_media_session_ptr __live_server_media_session;

        media_descriptor _descriptor;

        friend _rtsp_server_impl;
    };
}}}
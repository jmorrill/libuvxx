#pragma once
#include <unordered_map>
#include "event_dispatcher_object.hpp"
#include "media_descriptor.hpp"
#include "media_sample.hpp"
#include "FramedSource.hh"
#include "details/_live_framed_source.hpp"

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

        void on_framed_source_closed(int stream_id);

        void configure_session();

        FramedSource* create_framed_source(int stream_id, unsigned client_session_id);

    private:
        _live_server_media_session_ptr __live_server_media_session;

        media_descriptor _descriptor;

        std::unordered_map<int, std::shared_ptr<_live_framed_source>> _stream_sources;

        friend _rtsp_server_impl;
    };
}}}
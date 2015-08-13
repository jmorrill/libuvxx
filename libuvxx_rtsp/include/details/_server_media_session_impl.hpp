#pragma once
#include <unordered_map>
#include "event_dispatcher_object.hpp"
#include "media_descriptor.hpp"
#include "media_sample.hpp"
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

    using _streams_of_clients_map = std::unordered_map<int, std::unordered_map<unsigned, std::shared_ptr<_live_framed_source>>>;

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

        bool has_live_session();

        bool is_session_active();

    private:
        void on_session_closed();

        void on_framed_source_closed(int stream_id, unsigned client_session_id);

        void configure_session();

        void on_framed_source_created(int stream_id, unsigned client_session_id, const std::shared_ptr<_live_framed_source>& source);

    private:
        _live_server_media_session_ptr __live_server_media_session;

        media_descriptor _descriptor;

        _streams_of_clients_map _stream_sources;

        friend _rtsp_server_impl;
    };
}}}
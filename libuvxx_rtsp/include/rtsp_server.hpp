#pragma once
#include "event_dispatcher_object_base.hpp"
#include "server_media_session.hpp"

namespace uvxx { namespace rtsp { namespace details
{
    /* implementation forward */
    class _rtsp_server_impl;

    using _rtsp_server_impl_ptr = std::shared_ptr<_rtsp_server_impl>;
}}}

namespace uvxx { namespace rtsp 
{
    using on_session_request_delegate = std::function<uvxx::pplx::task<server_media_session>(const std::string& stream_name)>;

    class rtsp_server : public event_dispatcher_object_base<details::_rtsp_server_impl>
    {
    public:
        explicit rtsp_server();

        rtsp_server(const rtsp_server& client) = default;

        virtual rtsp_server& operator=(const rtsp_server&) = default;
    
    public:
        uint16_t port() const;

        void start_server(uint16_t port);

        void on_session_request_set(on_session_request_delegate callback);
    };
}}
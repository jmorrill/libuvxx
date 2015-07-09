#pragma once
#include "event_dispatcher_object.hpp"
#include "media_session.hpp"
#include "streaming_media_session.hpp"
#include "rtsp_misc.hpp"
#include <functional>

namespace uvxx { namespace rtsp { namespace details
{
    /* implementation forward */
    class _rtsp_client_impl;

    using _rtsp_client_impl_ptr = std::shared_ptr<_rtsp_client_impl>;
}}}

namespace uvxx { namespace rtsp 
{
    using read_stream_delegate = std::function<bool(const media_sample&)>;

    class rtsp_client : public uvxx::event_dispatcher_object
    {
    public:
        rtsp_client();

        rtsp_client(const rtsp_client&) = default;

        rtsp_client& operator=(const rtsp_client&) = default;

        rtsp_client(rtsp_client&& rhs);

        rtsp_client& operator=(rtsp_client&& rhs);

    public:
        uvxx::pplx::task<void> open(const std::string& url) const;

        uvxx::pplx::task<void> play() const;

        uvxx::pplx::task<void> play(std::vector<media_subsession> media_subsessions) const;

        void begin_stream_read(read_stream_delegate call_back) const;

        media_session media_session() const;

        std::string username() const;

        void username_set(const std::string& username);

        std::string password() const;

        void password_set(const std::string& password);

        uvxx::rtsp::transport_protocol protocol() const;

        void protocol_set(uvxx::rtsp::transport_protocol protocol);

    private:
        details::_rtsp_client_impl_ptr __rtsp_client_imp;

    };
}}
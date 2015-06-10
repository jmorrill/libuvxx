#pragma once
#include "event_dispatcher_object.hpp"
#include "media_session.hpp"
#include "streaming_media_session.hpp"

namespace uvxx { namespace rtsp { namespace details
{
    /* implementation forward */
    class _rtsp_client_impl;

    using _rtsp_client_impl_ptr = std::shared_ptr<_rtsp_client_impl>;
}}}

namespace uvxx { namespace rtsp 
{
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

        uvxx::pplx::task<streaming_media_session> play() const;

        uvxx::pplx::task<streaming_media_session> play(const std::vector<media_subsession>& media_subsessions) const;

        media_session media_session_get() const;

    private:
        details::_rtsp_client_impl_ptr __rtsp_client_imp;

    };
}}
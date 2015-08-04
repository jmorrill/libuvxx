#pragma once
#include "event_dispatcher_object.hpp"

namespace uvxx { namespace rtsp { namespace details
{
    /* implementation forward */
    class _server_media_session_impl;

    class _rtsp_server_impl;

    using _server_media_session_impl_ptr = std::shared_ptr<_server_media_session_impl>;
}}}

namespace uvxx { namespace rtsp 
{
    class server_media_session : public event_dispatcher_object
    {
    public:
        server_media_session();

        server_media_session(const server_media_session& client) = default;

        virtual server_media_session& operator=(const server_media_session&) = default;
   
        virtual ~server_media_session() = default;

    private:
        details::_server_media_session_impl_ptr __server_media_session_impl;

        friend details::_rtsp_server_impl;
    };
}}
#pragma once
#include "event_dispatcher_object.hpp"

namespace uvxx { namespace rtsp { namespace details
{
    /* implementation forwards */

    class _streaming_media_session_impl;

    using _streaming_media_session_impl_ptr = std::shared_ptr<_streaming_media_session_impl>;

}}}

namespace uvxx { namespace rtsp 
{
    class streaming_media_session : uvxx::event_dispatcher_object
    {
    public:
        streaming_media_session();

        streaming_media_session(const details::_streaming_media_session_impl_ptr& streaming_media_session_impl);

        streaming_media_session(const streaming_media_session&) = default;

        streaming_media_session& operator=(const streaming_media_session&) = default;

        streaming_media_session(streaming_media_session&& rhs);

        streaming_media_session& operator=(streaming_media_session&& rhs);

    private:
        details::_streaming_media_session_impl_ptr __streaming_media_session;
    };
}}
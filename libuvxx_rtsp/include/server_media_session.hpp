#pragma once
#include "event_dispatcher_object.hpp"

#include "media_descriptor.hpp"

namespace uvxx { namespace rtsp { namespace details
{
    /* implementation forward */
    class _server_media_session_impl;

    class _rtsp_server_impl;

    using _server_media_session_impl_ptr = std::shared_ptr<_server_media_session_impl>;
}}}

namespace uvxx { namespace rtsp 
{
    class media_sample;

    class server_media_session : public event_dispatcher_object
    {
    public:
        server_media_session();

        server_media_session(const server_media_session& client);

        server_media_session& operator=(const server_media_session&);
   
        server_media_session(server_media_session&& session);

        server_media_session& operator=(server_media_session&& session);

        bool operator !=(std::nullptr_t);

        bool operator==(std::nullptr_t)
        {
            if(!__server_media_session_impl)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        virtual ~server_media_session() = default;

    public:
        void set_media_descriptor(const media_descriptor& descriptor);

        void deliver_sample(int stream_id, const media_sample& sample);

    private:
        details::_server_media_session_impl_ptr __server_media_session_impl;

        friend details::_rtsp_server_impl;
    };
}}
#pragma once
#include "event_dispatcher_object.hpp"
#include "rtsp_misc.hpp"

namespace uvxx { namespace rtsp { namespace details
{
    /* implementation forward */
    class _rtsp_server_impl;

    using _rtsp_server_impl_ptr = std::shared_ptr<_rtsp_server_impl>;
}}}

namespace uvxx { namespace rtsp 
{
    class rtsp_server : public event_dispatcher_object
    {
    public:
        explicit rtsp_server();

        rtsp_server(const rtsp_server& client) = default;

        virtual rtsp_server& operator=(const rtsp_server&) = default;
    
    public:
        uint16_t port() const;

        void start_server(uint16_t port);

    private:
        details::_rtsp_server_impl_ptr __rtsp_server_impl;
    };
}}
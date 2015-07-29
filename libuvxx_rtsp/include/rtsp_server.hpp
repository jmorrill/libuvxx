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
        rtsp_server();

        rtsp_server(const rtsp_server& client);

        virtual rtsp_server& operator=(const rtsp_server&) = default;

        //rtsp_server(rtsp_server&& rhs);

        //virtual rtsp_server& operator=(rtsp_server&& rhs);
    public:
        
        int port_get() const;

        void start_server(int port);

    private:
        details::_rtsp_server_impl_ptr __rtsp_server_impl;
    };
}}
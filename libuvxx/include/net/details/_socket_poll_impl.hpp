#pragma once
#include <string>
#include "details/_event_dispatcher_timer_impl.hpp"
#include "details/_event_dispatcher_object_impl.hpp"
#include "details/_uv_poll.hpp"
#include "net/socket_poll.hpp"

namespace uvxx { namespace details
{ 
    class _event_dispatcher_object_impl;
}}

namespace uvxx { namespace net { namespace details
{
    class _socket_poll_impl : public uvxx::details::_event_dispatcher_object_impl
    {
    public:
        _socket_poll_impl(int socket);

        virtual ~_socket_poll_impl()
        {
            _poll.close();
        }

        void set_callback(std::function<void(int status, uvxx::net::socket_poll_event events)> callback);

        void start(int events);

        void stop();

    private:
        void poll_callback(int status, int events);

    private:
        std::function<void(int status, uvxx::net::socket_poll_event events)> _callback;

        uvxx::details::_uv_poll _poll;
    };
}}}
#pragma once
#include "global_misc.hpp"
#include <string>
#include "event_dispatcher_object.hpp"
#include "pplx/pplxtasks.h"
#include "io/memory_buffer.hpp"

namespace uvxx { namespace net { namespace details
{
    class _socket_poll_impl;
}}}

namespace uvxx { namespace net
{
    enum class socket_poll_event : int
    {
        Readable = 1,
        Writeable = 2
    };

    DEFINE_ENUM_FLAG(socket_poll_event)

    class socket_poll : public event_dispatcher_object
    {
    public:
        socket_poll(int socket);

        socket_poll(const socket_poll&) = default;

        socket_poll& operator=(const socket_poll&) = default;

        socket_poll(socket_poll&& rhs);

        socket_poll& operator=(socket_poll&& rhs);

        void set_callback(std::function<void(int status, uvxx::net::socket_poll_event events)> callback);

        void start(socket_poll_event events);

        void stop();

    private:
       std::function<void(int status, uvxx::net::socket_poll_event events)> _callback;

       std::shared_ptr<details::_socket_poll_impl> __socket_poll;
    };
}}
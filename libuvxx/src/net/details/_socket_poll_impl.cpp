#include "net/details/_socket_poll_impl.hpp"

using namespace uvxx::net;
using namespace uvxx::net::details;

_socket_poll_impl::_socket_poll_impl(int socket) : _poll(&dispatcher()->_loop,
    socket,
    std::bind(&_socket_poll_impl::poll_callback, this, std::placeholders::_1, std::placeholders::_2))
{

}

_socket_poll_impl::~_socket_poll_impl()
{
    _poll.close();
}

void _socket_poll_impl::start(int events)
{
    _poll.start(events);
}

void _socket_poll_impl::stop()
{
    _poll.stop();
}

void _socket_poll_impl::poll_callback(int status, int events)
{
    if (_callback)
    {
        _callback(status, static_cast<socket_poll_event>(events));
    }
}

void _socket_poll_impl::set_callback(std::function<void(int status, socket_poll_event events)> callback)
{
    _callback = callback;
}

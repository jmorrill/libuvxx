#include "details/_uv_poll.hpp"
#include "details/_uv_loop.hpp"

namespace uvxx { namespace details
{
    _uv_poll::_uv_poll(_uv_loop* uv_loop, int fd, std::function<void(int status, int events)> callback) : _uv_handle<uv_poll_t>(), 
                                                                                                          _callback(std::move(callback))
    {
        static_cast<uv_poll_t*>(*this)->loop = nullptr;
        uv_poll_init_socket(*uv_loop, *this, fd);
    }

    void _uv_poll::poll_callback(uv_poll_t * handle, int status, int events)
    {
        auto poll = static_cast<_uv_poll*>(handle->data);

        poll->_callback(status, events);
    }

    void _uv_poll::start(int events)
    {
        uv_poll_start(*this, events, poll_callback);
    }

    void _uv_poll::stop()
    {
        uv_poll_stop(*this);
    }
}}

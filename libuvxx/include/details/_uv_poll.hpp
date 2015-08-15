#pragma once
#include <functional>
#include "_uvxx_internal.hpp"
#include "_uv_handle.hpp"

namespace uvxx { namespace details
{
    class _uv_loop;
}}

namespace uvxx { namespace details
{
    class _uv_poll : public _uv_handle<uv_poll_t>
    {
    public:
        _uv_poll(_uv_loop* uv_loop, int fd, std::function<void(int status, int events)> callback);

        int start(int events);

        int stop();
    private:
        static void poll_callback(uv_poll_t * handle, int status, int events);

        std::function<void(int status, int events)> _callback;
    };
}}

#pragma once
#include <functional>
#include "_uvxx_internal.hpp"
#include "_uv_handle.hpp"

namespace uvxx { namespace details
{
    class _event_dispatcher_timer_impl;
}}

namespace uvxx { namespace details 
{
    class _uv_loop;

    class _uv_timer : public _uv_handle<uv_timer_t> 
    {

    public:
        _uv_timer(uvxx::details::_uv_loop*, std::function<void()> callback) ;

        bool start(uint64_t timeout, uint64_t repeat);

        bool stop();

    private:
        std::function<void()> _callback;

        uvxx::details::_uv_loop* _loop;

        static void timer_callback(uv_timer_t *req);
    };
}}

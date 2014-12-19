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
    class _uv_async  : public _uv_handle<uv_async_t> 
    {
    public:
        _uv_async(_uv_loop* uv_loop, std::function<void()> callback);

	    bool send();
    private:
        static void async_callback(uv_async_t* handle);

        std::function<void()> _callback;
    };
}} 
#pragma once
#include "_uvxx_internal.hpp"
#include "_uv_handle.hpp"
#include <functional>

namespace uvxx { namespace details
{
    class _uv_loop;
}}

namespace uvxx { namespace details
{
    class _uv_work : public _uv_handle<uv_work_t>
    {
     public:
        _uv_work(_uv_loop* uv_loop, std::function<void()> work_function);

        ~_uv_work();

     private:
        std::function<void()> _work_function;

        static void work_callback(uv_work_t* handle);

        static void work_callback_after(uv_work_t* handle, int status);
    };
}}
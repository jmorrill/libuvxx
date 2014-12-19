#pragma once
#include <functional>
#include <vector>
#include <thread>
#include <mutex>
#include "_uvxx_internal.hpp"
#include "_uv_async.hpp"

namespace uvxx { namespace details
{
    class _uv_loop
    {
    public:
        _uv_loop(bool use_default = false);

        ~_uv_loop();

        _uv_loop(const _uv_loop&) = delete;

        _uv_loop& operator=(const _uv_loop&) = delete;

        _uv_loop(_uv_loop&& other) = delete;

        _uv_loop& operator=(_uv_loop&& other) = delete;
   
        operator uv_loop_t*()
        {
            return this->__uv_loop_;
        }

        operator const uv_loop_t*() const
        {
            return this->__uv_loop_;
        }

        uv_loop_t* get() { return __uv_loop_; }

        bool run();

        bool run_once();

        bool run_nowait();

        void queue_work(std::function<void()> work_function);

        void begin_invoke(std::function<void()> operation);

        void begin_invoke_unsafe(std::function<void()> operation);

        void update_time() { uv_update_time(__uv_loop_); }

        int64_t now() { return uv_now(__uv_loop_); }

    private:
        uv_loop_t* __uv_loop_;

        std::vector<std::function<void()>> _begin_invoke_callbacks_unsafe;

        std::vector<std::function<void()>> _begin_invoke_callbacks;

        std::vector<std::function<void()>> _work_callbacks;

        uvxx::details::_uv_async _methods_to_invoke_async;

        uvxx::details::_uv_async _methods_to_invoke_async_unsafe;

        uvxx::details::_uv_async _work_methods_to_invoke_async;

        std::mutex _mutex;

        std::thread::id _thread_id;
    private:
        void on_process_invokes();

        void on_process_invokes_unsafe();

        void on_timer_tick();
    };
}}


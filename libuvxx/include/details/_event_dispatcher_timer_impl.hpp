#pragma once
#include <memory>
#include <chrono>
#include <vector>
#include "_event_dispatcher_object_impl.hpp"
#include "delegate.hpp"
#include "details/_uv_timer.hpp"
#include "pplx/pplxtasks.h"

namespace uvxx { namespace details
{
    class _event_dispatcher_timer_impl : public _event_dispatcher_object_impl,
                                         public std::enable_shared_from_this<_event_dispatcher_timer_impl>
    {
    public:
        _event_dispatcher_timer_impl();

        ~_event_dispatcher_timer_impl();

        bool is_enabled();

        void start();

        void stop();

        MULTICAST_EVENT(tick_event, _event_dispatcher_timer_impl)

        std::chrono::milliseconds timeout_get();

        void timeout_set(std::chrono::milliseconds timeout);

        std::chrono::milliseconds repeat_get();

        void repeat_set(std::chrono::milliseconds timeout);

        uvxx::pplx::task<void> delay(std::chrono::milliseconds timeout);

    private:
        void tick_callback();

    private:
        std::vector<uvxx::pplx::task_completion_event<void>> _task_events;

        _uv_timer _timer;

        int64_t _timer_timeout;

        int64_t _timer_repeat;

        bool _isEnabled;
    };
}}
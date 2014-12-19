#pragma once
#include <memory>
#include "event_dispatcher_object.hpp"
#include "delegate.hpp"
#include "pplx/pplxtasks.h"

namespace uvxx { namespace details
{
    /* implementation forward */
    class _event_dispatcher_timer_impl;
}}

namespace uvxx
{
    class event_dispatcher_timer : public event_dispatcher_object
    {
    public:
        event_dispatcher_timer();

        ~event_dispatcher_timer();

        /*event_dispatcher_timer(const event_dispatcher_timer&) = default;

        event_dispatcher_timer& operator=(const event_dispatcher_timer&) = default;*/

        event_dispatcher_timer(event_dispatcher_timer&& dispatcher);

        event_dispatcher_timer& operator=(event_dispatcher_timer&& rhs);

        operator bool() const;

        bool is_enabled();

        void start();

        void stop();

        MULTICAST_EVENT(tick_event, event_dispatcher_timer)
        
        std::chrono::milliseconds timeout_get();

        void timeout_set(std::chrono::milliseconds timeout);

        std::chrono::milliseconds repeat_get();

        void repeat_set(std::chrono::milliseconds timeout);

        uvxx::pplx::task<void> delay(std::chrono::milliseconds timeout) const;

    private:
        void tick_callback(void * sender);

        event_token _impl_token;
        std::shared_ptr<uvxx::details::_event_dispatcher_timer_impl> __event_dispatcher_timer_impl;
    };
}
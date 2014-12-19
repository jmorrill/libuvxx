#pragma once
#include "event_dispatcher_object.hpp"

namespace uvxx { namespace details
{
    /* implementation forward */
    class _event_dispatcher_frame_impl;
}}

namespace uvxx
{
    class event_dispatcher;

    class event_dispatcher_frame : public event_dispatcher_object
    {
    public:
        event_dispatcher_frame();

        event_dispatcher_frame(bool exitWhenRequested);

        event_dispatcher_frame(const event_dispatcher_frame&) = default;

        event_dispatcher_frame& operator=(const event_dispatcher_frame&) = default;

        event_dispatcher_frame(event_dispatcher_frame&& dispatcher);

        event_dispatcher_frame& operator=(event_dispatcher_frame&& rhs);

        operator bool() const;

        bool continue_get();

        void continue_set(bool shouldContinue);
    private:
        std::shared_ptr<uvxx::details::_event_dispatcher_frame_impl> __event_dispatcher_frame_impl;
        friend event_dispatcher;
    };
}

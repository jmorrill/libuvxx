#pragma once
#include <memory>
#include "_event_dispatcher_object_impl.hpp"


namespace uvxx { namespace details
{
    class _event_dispatcher_frame_impl;

    using event_dispatcher_frame_ptr = std::shared_ptr<_event_dispatcher_frame_impl>;

    class _event_dispatcher_frame_impl : public _event_dispatcher_object_impl
    {
    public:
        _event_dispatcher_frame_impl();

        _event_dispatcher_frame_impl(bool exitWhenRequested);

        bool get_continue();

        void set_continue(bool shouldContinue);

    private:
        bool _exitWhenRequested;
        bool _continue;

    };
}}
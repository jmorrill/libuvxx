#pragma once
#include <memory>
#include "_event_dispatcher_impl.hpp"

namespace uvxx
{
    /* friend forwards */
    class event_dispatcher_object;
    class event_dispatcher;
}

namespace uvxx { namespace details
{
    class _event_dispatcher_impl;
    using event_dispatcher_ptr = std::shared_ptr<_event_dispatcher_impl>;

    class _event_dispatcher_frame_impl;

    class _event_dispatcher_object_impl
    {
    public:
        _event_dispatcher_object_impl(const _event_dispatcher_object_impl&) = delete;

        _event_dispatcher_object_impl& operator=(const _event_dispatcher_object_impl&) = delete;

        bool check_access() const;

        void verify_access() const;

        event_dispatcher_ptr dispatcher();

        virtual ~_event_dispatcher_object_impl();

    protected:
        _event_dispatcher_object_impl();

    private:
        event_dispatcher_ptr _eventDispatcher;

        friend _event_dispatcher_frame_impl;

        friend event_dispatcher_object;
    };
}}

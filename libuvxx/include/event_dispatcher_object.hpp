#pragma once
#include <memory>
#include "event_dispatcher.hpp"

namespace uvxx { namespace details
{
    /* Implementation forward */
    class _event_dispatcher_object_impl;
}}

namespace uvxx
{
    /* friend forwards */
    class event_dispatcher_frame;

    class event_dispatcher_object
    {
    public:
        event_dispatcher_object(const event_dispatcher_object&) = default;

        event_dispatcher_object& operator=(const event_dispatcher_object&) = default;

        event_dispatcher_object(event_dispatcher_object&& dispatcher);

        event_dispatcher_object& operator=(event_dispatcher_object&& rhs);

        virtual operator bool() const;

        bool check_access();

        uvxx::event_dispatcher dispatcher();

        virtual ~event_dispatcher_object();

    protected:
        event_dispatcher_object();

        void verify_access() const;

    private:
        uvxx::event_dispatcher _event_dispatcher;

        std::shared_ptr<uvxx::details::_event_dispatcher_object_impl> __event_dispatcher_object_impl;

        friend event_dispatcher_frame;
    };
}

#include "details/_event_dispatcher_impl.hpp"
#include "details/_event_dispatcher_object_impl.hpp"
#include "event_dispatcher_object.hpp"

using namespace uvxx::details;

namespace uvxx
{
    event_dispatcher_object::event_dispatcher_object()
    {
        /* fix the 'new' usage */
        __event_dispatcher_object_impl = std::shared_ptr<_event_dispatcher_object_impl>(new _event_dispatcher_object_impl);
    }

    event_dispatcher_object::event_dispatcher_object(event_dispatcher_object&& dispatcher)
    {
        *this = std::move(dispatcher);
    }
    
    event_dispatcher_object& event_dispatcher_object::operator=(event_dispatcher_object&& rhs)
    {
        __event_dispatcher_object_impl = std::move(rhs.__event_dispatcher_object_impl);
        return *this;
    }

    event_dispatcher_object::~event_dispatcher_object()
    {

    }

    bool event_dispatcher_object::check_access()
    {
        return __event_dispatcher_object_impl->check_access();
    }

    event_dispatcher event_dispatcher_object::dispatcher()
    {
        event_dispatcher dispatcher;
        dispatcher.__event_dispatcher_impl = __event_dispatcher_object_impl->dispatcher();
        return dispatcher;
    }

    void event_dispatcher_object::verify_access() const
    {
        __event_dispatcher_object_impl->verify_access();
    }

    event_dispatcher_object::operator bool() const
    {
        return __event_dispatcher_object_impl.operator bool();
    }

}
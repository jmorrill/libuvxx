#include "details/_event_dispatcher_impl.hpp"
#include "details/_event_dispatcher_object_impl.hpp"

namespace uvxx { namespace details
{
    _event_dispatcher_object_impl::~_event_dispatcher_object_impl()
    {

    }

    _event_dispatcher_object_impl::_event_dispatcher_object_impl()
    {
        _eventDispatcher = _event_dispatcher_impl::current_dispatcher();
    }

    bool _event_dispatcher_object_impl::check_access()
    {
        return _eventDispatcher->check_access();
    }

    _event_dispatcher_impl_ptr _event_dispatcher_object_impl::dispatcher()
    {
        return _eventDispatcher;
    }

    void _event_dispatcher_object_impl::verify_access()
    {
        _eventDispatcher->verify_access();
    }
}}
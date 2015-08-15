#include "pplx/pplxtasks.h"
#include "details/_event_dispatcher_impl.hpp"

namespace uvxx { namespace pplx { namespace details 
{
    void begin_invoke_on_current(std::function<void()> method)
    {
        auto dispatcher = uvxx::details::_event_dispatcher_impl::current_dispatcher();

        dispatcher->begin_invoke_impl_unsafe(std::move(std::move(method)));
    }
    
    bool _ContextCallback::_IsCurrentOriginSTA()
    {
        auto has_dispatcher = uvxx::details::_event_dispatcher_impl::has_dispatcher();
    
        return has_dispatcher;
    }
    
    void _ContextCallback::_Capture()
    {
        if (_M_context._M_captureMethod != _S_captureDeferred)
        {
            return;
        }

         _M_context._M_pContextCallback = uvxx::details::_event_dispatcher_impl::this_thread_id();
    }

    void _ContextCallback::_CallInContext(_CallbackFunction _Func) const
    {
        if (!_HasCapturedContext())
        {
            _Func();
        }
        else
        {
            auto dispatcher = uvxx::details::_event_dispatcher_impl::from_thread(_M_context._M_pContextCallback);

            if (dispatcher)
            {
                if (dispatcher->thread_id() == uvxx::details::_event_dispatcher_impl::this_thread_id())
                {
                    dispatcher->begin_invoke_impl_unsafe(std::move(_Func));
                }
                else
                {
                    dispatcher->begin_invoke_impl(std::move(_Func));
                }
            }
            else
            {
                _Func();
            }
        }
    }

    _ContextCallback::_ContextCallback(bool _DeferCapture /*= false*/)
    {
        if (_DeferCapture)
        {
            _M_context._M_captureMethod = _S_captureDeferred;
        }
        else
        {
            _M_context._M_pContextCallback = std::thread::id();
        }
    }

    bool _Task_impl_base::_IsNonBlockingThread()
    {
        auto has_dispatcher = uvxx::details::_event_dispatcher_impl::has_dispatcher();
    
        return has_dispatcher;
    }

    bool _Task_impl_base::_IsApartmentAware()
    {
        return _M_fFromAsync;
    }

    bool _HasDispatcher()
    {
        return uvxx::details::_event_dispatcher_impl::has_dispatcher();
    }

}}}

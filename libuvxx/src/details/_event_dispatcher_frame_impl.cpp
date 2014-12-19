#include "details/_event_dispatcher_frame_impl.hpp"

namespace uvxx { namespace details
{
    _event_dispatcher_frame_impl::_event_dispatcher_frame_impl() : _event_dispatcher_frame_impl(true)
    {

    }

    _event_dispatcher_frame_impl::_event_dispatcher_frame_impl(bool exitWhenRequested)
    {
        _exitWhenRequested = exitWhenRequested;
        _continue = true;
    }

    bool _event_dispatcher_frame_impl::get_continue()
    {
        bool shouldContinue = _continue;

        if (shouldContinue)
        {
            if (_exitWhenRequested)
            {
                if (_eventDispatcher->_exitAllFrames || 
                    _eventDispatcher->_hasShutdownStarted)
                {
                    shouldContinue = false;
                }
            }
        }

        return shouldContinue;
    }

    void _event_dispatcher_frame_impl::set_continue(bool shouldContinue)
    {
        _continue = shouldContinue;

        /* pump the event loop */
        _eventDispatcher->begin_invoke([]{});
    }
}}
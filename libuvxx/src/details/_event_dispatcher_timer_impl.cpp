#include "details/_event_dispatcher_impl.hpp"
#include "details/_event_dispatcher_timer_impl.hpp"
#include "details/_uv_loop.hpp"
#include "delegate.hpp"
#include "details/_uv_timer.hpp"

using namespace std;
using namespace uvxx::pplx;

namespace uvxx { namespace details
{
    _event_dispatcher_timer_impl::_event_dispatcher_timer_impl()  :
        _timer(&dispatcher()->_loop, bind(&_event_dispatcher_timer_impl::tick_callback, this)),
        _timer_timeout(0), 
        _timer_repeat(0),
        _isEnabled(false)
    {
        
    }

    void _event_dispatcher_timer_impl::tick_callback()
    {
        tick_event().invoke(this);

        for (auto& e : _task_events)
        {
            e.set();
        }

        _task_events.clear();
    }

    _event_dispatcher_timer_impl::~_event_dispatcher_timer_impl()
    {
        _timer.close();
    }

    void _event_dispatcher_timer_impl::start()
    {
         verify_access();

        _isEnabled = true;

        _timer.start(_timer_timeout, _timer_repeat);
    }

    void _event_dispatcher_timer_impl::stop()
    {
         verify_access();

        _isEnabled = false;

        _timer.stop();
    }

    chrono::milliseconds _event_dispatcher_timer_impl::timeout_get()
    {
        return chrono::milliseconds(_timer_timeout);
    }

    void _event_dispatcher_timer_impl::timeout_set(chrono::milliseconds timeout)
    {
         verify_access();

        _timer_timeout = timeout.count();

        if (_isEnabled)
        {
            start();
        }
    }

    chrono::milliseconds _event_dispatcher_timer_impl::repeat_get()
    {
        return chrono::milliseconds(_timer_repeat);
    }

    void _event_dispatcher_timer_impl::repeat_set(chrono::milliseconds timeout)
    {
         verify_access();

        _timer_repeat = timeout.count();

        if (_isEnabled)
        {
            start();
        }
    }

    bool _event_dispatcher_timer_impl::is_enabled()
    {
        return _isEnabled;
    }

    task<void> _event_dispatcher_timer_impl::delay(std::chrono::milliseconds timeout)
    {
        verify_access();

        task_completion_event<void> evt;

        _task_events.push_back(evt);

        timeout_set(timeout);

        start();

        auto this_ptr = shared_from_this();

        return task<void>(evt).then([this_ptr]
        {
            /* stop is not required, but added in case the compiler
                tried gettin' smart with this */
            this_ptr->stop();
        });
    }
}}
#include "details/_event_dispatcher_impl.hpp"
#include "details/_event_dispatcher_timer_impl.hpp"
#include "event_dispatcher_timer.hpp"
#include "delegate.hpp"

using namespace std;
using namespace uvxx::pplx;

namespace uvxx
{
    event_dispatcher_timer::event_dispatcher_timer() : 
        __event_dispatcher_timer_impl(std::make_shared<details::_event_dispatcher_timer_impl>())
    {
         _impl_token = __event_dispatcher_timer_impl->tick_event() += std::bind(&event_dispatcher_timer::tick_callback, this, placeholders::_1);
    }

    event_dispatcher_timer::event_dispatcher_timer(event_dispatcher_timer&& dispatcher_timer) : 
        event_dispatcher_object(std::move(dispatcher_timer))
    {
    }
    
    event_dispatcher_timer& event_dispatcher_timer::operator=(event_dispatcher_timer&& rhs)
    {
        __event_dispatcher_timer_impl = std::move(rhs.__event_dispatcher_timer_impl);

        return *this;
    }

    void event_dispatcher_timer::tick_callback(void * sender)
    {
        this->_tick_event.invoke(this);
    }

    event_dispatcher_timer::~event_dispatcher_timer()
    {
       __event_dispatcher_timer_impl->tick_event() -= _impl_token;
    }

    void event_dispatcher_timer::start()
    {
        __event_dispatcher_timer_impl->start();
    }

    void event_dispatcher_timer::stop()
    {
        __event_dispatcher_timer_impl->stop();
    }

    chrono::milliseconds event_dispatcher_timer::timeout_get()
    {
        return __event_dispatcher_timer_impl->timeout_get();
    }

    void event_dispatcher_timer::timeout_set(chrono::milliseconds timeout)
    {
        __event_dispatcher_timer_impl->timeout_set(timeout);
    }

    chrono::milliseconds event_dispatcher_timer::repeat_get()
    {
        return __event_dispatcher_timer_impl->repeat_get();
    }

    void event_dispatcher_timer::repeat_set(chrono::milliseconds timeout)
    {
        __event_dispatcher_timer_impl->repeat_set(timeout);
    }

    bool event_dispatcher_timer::is_enabled()
    {
        return __event_dispatcher_timer_impl->is_enabled();
    }

    task<void> event_dispatcher_timer::delay(std::chrono::milliseconds timeout) const
    {
        return __event_dispatcher_timer_impl->delay(timeout);
    }

    event_dispatcher_timer::operator bool() const
    {
        return __event_dispatcher_timer_impl.operator bool();
    }
}
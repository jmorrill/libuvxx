#include "details/_event_dispatcher_impl.hpp"
#include "details/_event_dispatcher_frame_impl.hpp"
#include "event_dispatcher.hpp"
#include "event_dispatcher_frame.hpp"

using namespace uvxx::details;

namespace uvxx
{
    event_dispatcher::event_dispatcher()
    {

    }

    event_dispatcher::event_dispatcher(event_dispatcher&& dispatcher)
    {
        *this = std::move(dispatcher);
    }

    event_dispatcher::event_dispatcher(const event_dispatcher& rhs) : __event_dispatcher_impl(rhs.__event_dispatcher_impl)
    {

    }


    event_dispatcher& event_dispatcher::operator=(event_dispatcher&& rhs)
    {
        __event_dispatcher_impl = std::move(rhs.__event_dispatcher_impl);

        return *this;
    }

    event_dispatcher& event_dispatcher::operator=(const event_dispatcher& rhs)
    {
        __event_dispatcher_impl = rhs.__event_dispatcher_impl;
        return *this;
    }

    event_dispatcher::~event_dispatcher()
    {

    }

    void event_dispatcher::run()
    {
        _event_dispatcher_impl::run();
    }

    void event_dispatcher::exit_all_frames()
    {
        _event_dispatcher_impl::exit_all_frames();
    }

    void event_dispatcher::push_frame(event_dispatcher_frame const& frame)
    {
        _event_dispatcher_impl::push_frame(frame.__event_dispatcher_frame_impl);
    }

    uvxx::event_dispatcher event_dispatcher::current_dispatcher()
    {
        event_dispatcher dispatcher;

        dispatcher.__event_dispatcher_impl = std::move(_event_dispatcher_impl::current_dispatcher());

        return dispatcher;
    }

    uvxx::event_dispatcher event_dispatcher::from_thread(std::thread::id const& thread_id)
    {
        event_dispatcher dispatcher;

        dispatcher.__event_dispatcher_impl = std::move(_event_dispatcher_impl::from_thread(thread_id));

        return dispatcher;
    }

    void event_dispatcher::begin_shutdown()
    {
        __event_dispatcher_impl->begin_shutdown();
    }

    bool event_dispatcher::check_access()
    {
        return __event_dispatcher_impl->check_access();
    }

    void event_dispatcher::verify_access()
    {
        __event_dispatcher_impl->verify_access();
    }

    std::thread::id const & event_dispatcher::thread_id()
    {
        return __event_dispatcher_impl->thread_id();
    }

    void event_dispatcher::begin_invoke_impl(std::function<void()> callback)
    {
        __event_dispatcher_impl->begin_invoke_impl(std::move(callback));
    }

    event_dispatcher::operator bool() const
    {
        return __event_dispatcher_impl.operator bool();
    }
}

#pragma once
#include <memory>
#include <thread>
#include <functional>
#include "pplx/pplxtasks.h"

namespace uvxx { namespace details
{
    /* implementation forward */
    class _event_dispatcher_impl;
}}

namespace uvxx 
{
    /* friend forwards */
    class event_dispatcher_object;
    class event_dispatcher_frame;

    class event_dispatcher
    {
    public:
        ~event_dispatcher();

        event_dispatcher(const event_dispatcher& rhs);

        event_dispatcher& operator=(const event_dispatcher& rhs);

        event_dispatcher(event_dispatcher&& dispatcher);

        event_dispatcher& operator=(event_dispatcher&& rhs);

        operator bool() const;

        static void run();

        static void exit_all_frames();

        static void push_frame(event_dispatcher_frame const& frame);

        static event_dispatcher current_dispatcher();

        static event_dispatcher from_thread(std::thread::id const& thread_id);

        void begin_shutdown();

        bool check_access();

        void verify_access();

        template <typename T>
        uvxx::pplx::task<decltype(std::declval<T>()())> 
        inline begin_invoke(T fun, typename std::enable_if<!std::is_void< decltype( std::declval<T>()() ) >::value, T>::type* = 0)  
        {
            uvxx::pplx::task_completion_event<decltype(fun())> evt;

            std::function<void()> wrapped_callback = [fun, evt]()
            {
                try
                {
                    auto ret = fun();
                    evt.set(ret);
                }
                catch (std::exception const& e)
                {
                    evt.set_exception(e);
                }
            };

            begin_invoke_impl(std::move(wrapped_callback));
            return pplx::task<decltype(fun())>(std::move(evt));
        }

        pplx::task<void> begin_invoke(std::function<void()> callback)  
        {
            uvxx::pplx::task_completion_event<void> task_event;
            auto invoke_task = create_task(task_event);

            auto task_capture = [callback = std::move(callback), task_event = std::move(task_event)]()
            {
                try
                {
                    callback();
                    task_event.set();
                }
                catch (std::exception const& e)
                {
                    task_event.set_exception(e);
                }
            };
           
            begin_invoke_impl(std::move(task_capture));
            return invoke_task;
        }

        std::thread::id const & thread_id();

        static bool has_dispatcher();

    private:
        event_dispatcher();

        void begin_invoke_impl(std::function<void()> callback);

    private:
        std::shared_ptr<uvxx::details::_event_dispatcher_impl> __event_dispatcher_impl;

        friend uvxx::event_dispatcher_frame;

        friend uvxx::event_dispatcher_object;
    };
}

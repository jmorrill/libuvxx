#pragma once

#ifndef _EVENT_DISPATCHER_HPP
#define _EVENT_DISPATCHER_HPP

#if !defined(_WIN32) 
#undef __cplusplus
#define __cplusplus 201103L
#endif

#include <memory>
#include <thread>
#include <vector>
#include <mutex>
#include <functional>

#include "details/_uv_loop.hpp"
#include "pplx/pplxtasks.h"

namespace uvxx 
{ 
    class event_dispatcher;

    namespace net { namespace details
    {    
        class _stream_socket_impl;
        class _dns_impl;
    }}

    namespace fs { namespace details
    {    
        class _file_impl;
        class _directory_impl;
    }}
}

namespace uvxx { namespace pplx { namespace details
{
    class _ContextCallback;
}}}

namespace crossplat
{
    class threadpool;
}

namespace uvxx { namespace details
{
    class _event_dispatcher_impl;
    using  event_dispatcher_ptr = std::shared_ptr<_event_dispatcher_impl>;
    using  _event_dispatcher_impl_weak_ptr = std::weak_ptr<_event_dispatcher_impl>;

    class _event_dispatcher_timer_impl;
    class _event_dispatcher_frame_impl;
    using event_dispatcher_frame_ptr = std::shared_ptr<_event_dispatcher_frame_impl>;

    class _event_dispatcher_impl
    {
    protected:
        ~_event_dispatcher_impl();
    public:
        _event_dispatcher_impl();

        _event_dispatcher_impl(const _event_dispatcher_impl&) = delete;

        _event_dispatcher_impl& operator=(const _event_dispatcher_impl&) = delete;

        static void run();

        static void exit_all_frames();
        
        static void push_frame(event_dispatcher_frame_ptr const& frame);

        static event_dispatcher_ptr current_dispatcher();

        static event_dispatcher_ptr from_thread(std::thread::id const& thread_id);

        void begin_shutdown();

        bool check_access() const;

        void verify_access() const;

        template <typename T>
        pplx::task<decltype(std::declval<T>()())> 
        begin_invoke(T fun, typename std::enable_if<!std::is_void< decltype( std::declval<T>()() ) >::value, T>::type* = 0)  
        {
            uvxx::pplx::task_completion_event<decltype(fun())> evt;

            std::function<void()> wrapped_callback = [fun, evt]()
            {
                try
                {
                    auto ret = fun();
                    evt.set(ret);
                }
                catch (std::exception& e)
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

#if defined(_MSC_VER)
            auto task_capture = [callback, task_event]()
#else
            auto task_capture = [callback(std::move(callback)), task_event(std::move(task_event))]()
#endif
            {
                try
                {
                    callback();
                    task_event.set();
                }
                catch (std::exception& e)
                {
                    task_event.set_exception(e);
                }
            };
           
            begin_invoke_impl(std::move(task_capture));
            return invoke_task;
        }

        std::thread::id const & thread_id() const;

        int frame_depth() const;

        static bool has_dispatcher();

        static std::thread::id this_thread_id();

        uvxx::details::_uv_loop _loop;

    private:
        void begin_invoke_impl(std::function<void()> callback);

        void begin_invoke_impl_unsafe(std::function<void()> callback);

        void push_frame_impl(event_dispatcher_frame_ptr const & frame);

        void start_shutdown_impl();

        void shutdown_impl();

    private:

        static std::vector<_event_dispatcher_impl_weak_ptr>& _dispatchers();

        static std::mutex& _global_lock();

        std::thread::id _threadId;

        bool _exitAllFrames;
        bool _hasShutdownStarted;
        bool _startingShutdown;
        bool _hasShutdownFinished;
        bool _hasShutdown;
        int _frameDepth;

        friend event_dispatcher;
        friend _event_dispatcher_frame_impl;
        friend _event_dispatcher_timer_impl;
        friend uvxx::net::details::_stream_socket_impl;
        friend uvxx::net::details::_dns_impl;
        friend uvxx::fs::details::_file_impl;
        friend uvxx::fs::details::_directory_impl;
        friend ::crossplat::threadpool;
        friend uvxx::pplx::details::_ContextCallback;
        friend void uvxx::pplx::details::begin_invoke_on_current(std::function<void()>);
    };
}}
#endif
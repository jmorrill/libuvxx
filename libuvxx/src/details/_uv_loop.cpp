#include "details/_uv_async.hpp"
#include "details/_uv_loop.hpp"
#include "details/_uv_work.hpp"
#include <iostream>

namespace uvxx { namespace details
{
    _uv_loop::_uv_loop(bool use_default /*= false*/) :
        __uv_loop_(use_default ? uv_default_loop() : uv_loop_new()),
        _methods_to_invoke_async(this,       std::bind(&_uv_loop::on_process_invokes, this)),
        _methods_to_invoke_async_unsafe(this, std::bind(&_uv_loop::on_process_invokes_unsafe, this)),
        _work_methods_to_invoke_async(this,  std::bind(&_uv_loop::on_process_invokes, this)),
        _thread_id(std::this_thread::get_id())
    {
        __uv_loop_->data = this;
    }

    _uv_loop::~_uv_loop()
    {
        if (__uv_loop_)
        {
            _begin_invoke_callbacks.clear();

            _work_callbacks.clear();

            _begin_invoke_callbacks_unsafe.clear();

            _methods_to_invoke_async.close();

            _work_methods_to_invoke_async.close();

            _methods_to_invoke_async_unsafe.close();

            while (uv_loop_alive(__uv_loop_) != 0)
            {
                run_once();
            }

            uv_loop_delete(__uv_loop_);

            __uv_loop_ = nullptr;
        }
    }
    
    /* processes the 'unsafe' that avoids locking.*/
    /* this callback happens off the loop when 
     * _method_to_invoke_async_unsafe.send() has been flagged */
    void _uv_loop::on_process_invokes_unsafe()
    {
        if (_begin_invoke_callbacks_unsafe.empty())
        {
            return;
        }

        /* temporary vectors */
        std::vector<std::function<void()>> async_list;

        std::swap(_begin_invoke_callbacks_unsafe, async_list);
     
        /* process queued methods invokes */
        for (auto& fun : async_list)
        {
            fun();
        }
    }
    
    /* processes all work queued for this loop to execute.  this includes
     * queued thread pool operations (aka work) 
    /* this callback happens off the loop when 
     * _methods_to_invoke_async/_work_methods_to_invoke_async.send() has been flagged */
    void _uv_loop::on_process_invokes()
    {
        /* quit of there is nothing to do - should not happen */
        if (_begin_invoke_callbacks.empty() && 
            _work_callbacks.empty())
        {
            return;
        }

        /* temporary vectors */
        std::vector<std::function<void()>> async_list;

        std::vector<std::function<void()>> work_list;

        async_list.reserve(2);
        work_list.reserve(2);

        {
            std::lock_guard<std::mutex> lock(_mutex);

            /* move the invoke methods to a local vector, 
             * to avoid any rentrancy issues */
            std::swap(_begin_invoke_callbacks, async_list);

            std::swap(_work_callbacks, work_list);
        }

        /* process threadpool invokes */
        for (auto& fun : work_list)
        {
            /* queue the work on thread pool (uv_queue_work)
             * 'new' here cleans it self up */
            new _uv_work(this, std::move(fun));
        }

        /* process queued methods invokes */
        for (auto& fun : async_list)
        {
            fun();
        }
    }
   
    /* queues thread pool work to happen via this loop */
    void _uv_loop::queue_work(std::function<void()> work_function)
    {
        {
            std::lock_guard<std::mutex> lock(_mutex);
           
            _work_callbacks.emplace_back(std::move(work_function));
        }

        /* uv_async_send + uv_async_t - these libuv methods are thread-safe
        /* flags loop to look for threadpool invokes on next loop iteration*/
        _work_methods_to_invoke_async.send();
    }

    /* queues work to happen on this loop.  this method is thread safe */
    void _uv_loop::begin_invoke(std::function<void()> operation)
    {
        {
            /* lock our vector<function<void()> of work */
            std::lock_guard<std::mutex> lock(_mutex);
            
            /* add the work to our list of callbacks for this loop to execute */
            _begin_invoke_callbacks.emplace_back(std::move(operation));
        }

        /* uv_async_send + uv_async_t - these libuv methods are thread-safe
         * this flags the async callback to happen on the next run of the loop  */
        _methods_to_invoke_async.send();
    }

    /* non-thread safe impl of begin_invoke for when an event 
     * loop needs to queue work on it's own thread,
     * avoiding any lock overhead */
    void _uv_loop::begin_invoke_unsafe(std::function<void()> operation)
    {
        /* store this unsafe callback in a place separate from other queued work */
        _begin_invoke_callbacks_unsafe.emplace_back(std::move(operation));

        /* uv_async_send + uv_async_t - these libuv methods are thread-safe
         * this flags the async callback to happen on the next run of the loop  */
        _methods_to_invoke_async_unsafe.send();
    }

    bool _uv_loop::run()
    {
        return uv_run(__uv_loop_, UV_RUN_DEFAULT) == 0;
    }

    bool _uv_loop::run_once()
    {
        return uv_run(__uv_loop_, UV_RUN_ONCE) == 0;
    }

    bool _uv_loop::run_nowait()
    {
        return uv_run(__uv_loop_, UV_RUN_NOWAIT) == 0;
    }
}}
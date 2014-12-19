#pragma once
#if !defined(_MSC_VER) 
#undef __cplusplus
#define __cplusplus 201103L
#endif
#include <thread>
#include <vector>
#include <mutex>
#include <iostream>
#include "details/_uv_loop.hpp"
#include "details/_event_dispatcher_impl.hpp"

namespace crossplat 
{
    class threadpool
    {
    public:
        
         
        threadpool(size_t /*n*/) : _loop(false)
        {
            try
            {
                 std::thread t(threadpool::start_thread, this);
                 t.detach();
            }
            catch (...)
            {
            }
        }

        static threadpool& shared_instance()
        {
            return s_shared;
        }

        ~threadpool()
        {
       
        }

        template<typename T>
        void schedule(T task)
        {
            _loop.queue_work(std::move(task));
        }

    private:
        uvxx::details::_uv_loop _loop;

        static threadpool s_shared;

        static void start_thread(threadpool* thread_pool)
        {
            thread_pool->_loop.run();
        }
    };
}

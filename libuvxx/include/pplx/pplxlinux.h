/***
* ==++==
*
* Copyright (c) Microsoft Corporation. All rights reserved. 
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
* http://www.apache.org/licenses/LICENSE-2.0
* 
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
* ==--==
* =+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
*
* pplxlinux.h
*
* Linux specific pplx implementations
*
* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
****/

#pragma once

#ifndef _PPLXLINUX_H
#define _PPLXLINUX_H


#include <signal.h>
#include <mutex>
#include <condition_variable>

#if !defined(_MSC_VER) 
#include "pthread.h"
#define _noexcept noexcept
#else
#define _noexcept 
#endif

#include "compat/linux_compat.h"
#include "pplx/pplxinterface.h"


namespace uvxx { namespace pplx
{
namespace details
{
namespace platform
{
    /// <summary>
    /// Returns a unique identifier for the execution thread where this routine in invoked
    /// </summary>
    long GetCurrentThreadId();

    /// <summary>
    /// Yields the execution of the current execution thread - typically when spin-waiting
    /// </summary>
    void YieldExecution();

    /// <summary>
    /// Caputeres the callstack
    /// </summary>
    __declspec(noinline) inline static size_t CaptureCallstack(void **, size_t, size_t)
    {
        return 0;
    }
}

    /// <summary>
    /// Manual reset event
    /// </summary>
    class event_impl
     {
    private:
        std::mutex _lock;
        std::condition_variable _condition;
        bool _signaled;
    public:

        static const unsigned int timeout_infinite = 0xFFFFFFFF;

        event_impl()
            : _signaled(false) 
        {
        }

        void set()
        {
            std::lock_guard<std::mutex> lock(_lock);
            _signaled = true;
            _condition.notify_all();
        }

        void reset()
        {
            std::lock_guard<std::mutex> lock(_lock);
            _signaled = false;
        }

        unsigned int wait(unsigned int timeout)
        {
            std::unique_lock<std::mutex> lock(_lock);
            if (timeout == event_impl::timeout_infinite)
            {
                _condition.wait(lock, [this]() -> bool { return _signaled; });
                return 0;
            }
            else
            {
                std::chrono::milliseconds period(timeout);
                auto status = _condition.wait_for(lock, period, [this]() -> bool { return _signaled; });
                _ASSERTE(status == _signaled);
                // Return 0 if the wait completed as a result of signaling the event. Otherwise, return timeout_infinite
                // Note: this must be consistent with the behavior of the Windows version, which is based on WaitForSingleObjectEx
                return status ? 0: event_impl::timeout_infinite;
            }
        }

        unsigned int wait()
        {
            return wait(event_impl::timeout_infinite);
        }
    };


#if defined(_MSC_VER) 
    /// <summary>
    /// Reader writer lock
    /// </summary>
    class reader_writer_lock_impl
    {
    public:

        class scoped_lock_read
        {
        public:
            explicit scoped_lock_read(reader_writer_lock_impl &_Reader_writer_lock) : _M_reader_writer_lock(_Reader_writer_lock)
            {
                _M_reader_writer_lock.lock_read();
            }

            ~scoped_lock_read()
            {
                _M_reader_writer_lock.unlock();
            }

        private:
            reader_writer_lock_impl& _M_reader_writer_lock;
            scoped_lock_read(const scoped_lock_read&);                    // no copy constructor
            scoped_lock_read const& operator=(const scoped_lock_read&);  // no assignment operator
        };

        reader_writer_lock_impl();

        void lock();

        void lock_read();

        void unlock();

    private:

        // Windows slim reader writer lock
        void * _M_impl;

        // Slim reader writer lock doesn't have a general 'unlock' method.
        // We need to track how it was acquired and release accordingly.
        // true - lock exclusive
        // false - lock shared
        bool m_locked_exclusive;
    };  

#else

    /// <summary>
    /// Reader writer lock
    /// </summary>
    class reader_writer_lock_impl
    {
    private:

        pthread_rwlock_t _M_reader_writer_lock;

    public:

        class scoped_lock_read
        {
        public:
            explicit scoped_lock_read(reader_writer_lock_impl &_Reader_writer_lock) : _M_reader_writer_lock(_Reader_writer_lock)
            {
                _M_reader_writer_lock.lock_read();
            }

            ~scoped_lock_read()
            {
                _M_reader_writer_lock.unlock();
            }

        private:
            reader_writer_lock_impl& _M_reader_writer_lock;
            scoped_lock_read(const scoped_lock_read&);                    // no copy constructor
            scoped_lock_read const& operator=(const scoped_lock_read&);  // no assignment operator
        };

        reader_writer_lock_impl()
        {
            pthread_rwlock_init(&_M_reader_writer_lock, nullptr);
        }

        ~reader_writer_lock_impl()
        {
            pthread_rwlock_destroy(&_M_reader_writer_lock);
        }

        void lock()
        {
            pthread_rwlock_wrlock(&_M_reader_writer_lock);
        }

        void lock_read()
        {
            pthread_rwlock_rdlock(&_M_reader_writer_lock);
        }

        void unlock()
        {
            pthread_rwlock_unlock(&_M_reader_writer_lock);
        }
    };
#endif

    /// <summary>
    /// Recursive mutex
    /// </summary>
    class recursive_lock_impl
    {
    public:

        recursive_lock_impl()
            : _M_owner(-1), _M_recursionCount(0)
        {
        }

        ~recursive_lock_impl()
        {
            _ASSERTE(_M_owner == -1);
            _ASSERTE(_M_recursionCount == 0);
        }

        void lock()
        {
            auto id = uvxx::pplx::details::platform::GetCurrentThreadId();

            if ( _M_owner == id )
            {
                _M_recursionCount++;
            }
            else
            {
                _M_cs.lock();
                _M_owner = id;
                _M_recursionCount = 1;
            }            
        }

        void unlock()
        {
            _ASSERTE(_M_owner == uvxx::pplx::details::platform::GetCurrentThreadId());
            _ASSERTE(_M_recursionCount >= 1);

            _M_recursionCount--;

            if ( _M_recursionCount == 0 )
            {
                _M_owner = -1;
                _M_cs.unlock();
            }           
        }

    private:
        std::mutex _M_cs;
        long _M_recursionCount;
        volatile long _M_owner;
    };


    class linux_scheduler : public uvxx::pplx::scheduler_interface
    {
    public:
        virtual void schedule( TaskProc_t proc, _In_ void* param);
    };

} // namespace details

/// <summary>
///  A generic RAII wrapper for locks that implement the critical_section interface
///  std::lock_guard
/// </summary>
template<class _Lock>
class scoped_lock
{
public:
    explicit scoped_lock(_Lock& _Critical_section) : _M_critical_section(_Critical_section), _deleted(false)
    {
        _M_critical_section.lock();
    }

    ~scoped_lock()
    {
        if (_deleted)
        {
            return;
        }

        _deleted = true;
        _M_critical_section.unlock();
    }

private:
    _Lock& _M_critical_section;
    bool _deleted;
    scoped_lock(const scoped_lock&);                    // no copy constructor
    scoped_lock const& operator=(const scoped_lock&);  // no assignment operator
};

// The extensibility namespace contains the type definitions that are used internally
namespace extensibility
{
    typedef uvxx::pplx::details::event_impl event_t;

    typedef std::mutex critical_section_t;
    typedef scoped_lock<critical_section_t> scoped_critical_section_t;

    typedef uvxx::pplx::details::reader_writer_lock_impl reader_writer_lock_t;
    typedef scoped_lock<reader_writer_lock_t> scoped_rw_lock_t;
    typedef uvxx::pplx::extensibility::reader_writer_lock_t::scoped_lock_read scoped_read_lock_t;

    typedef uvxx::pplx::details::recursive_lock_impl recursive_lock_t;
    typedef scoped_lock<recursive_lock_t> scoped_recursive_lock_t;
}


typedef details::linux_scheduler default_scheduler_t;
    
namespace details
{

#if !defined(_MSC_VER)
    /// <summary>
    /// Terminate the process due to unhandled exception
    /// </summary>
    #ifndef _REPORT_PPLTASK_UNOBSERVED_EXCEPTION
    #define _REPORT_PPLTASK_UNOBSERVED_EXCEPTION() do { \
        raise(SIGTRAP); \
        std::terminate(); \
    } while(false)
    #endif //_REPORT_PPLTASK_UNOBSERVED_EXCEPTION
#else
    /// <summary>
    /// Terminate the process due to unhandled exception
    /// </summary>

    #ifndef _REPORT_PPLTASK_UNOBSERVED_EXCEPTION
    #define _REPORT_PPLTASK_UNOBSERVED_EXCEPTION() do { \
        __debugbreak(); \
        std::terminate(); \
    } while(false)
    #endif // _REPORT_PPLTASK_UNOBSERVED_EXCEPTION

#endif
}


#if !defined(_MSC_VER) 
//see: http://gcc.gnu.org/onlinedocs/gcc/Return-Address.html
// this is critical to inline
__attribute__ ((always_inline))
inline void* _ReturnAddress() { return __builtin_return_address(0); }
#else
inline void* _ReturnAddress(){ return nullptr; }
#endif

} }// namespace uvxx { namespace pplx

#endif // _PPLXLINUX_H

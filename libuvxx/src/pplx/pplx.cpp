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
* pplx.cpp
*
* Parallel Patterns Library implementation (common code across platforms)
*
* For the latest on this and related APIs, please see http://casablanca.codeplex.com.
*
* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
****/

#include "pplx/pplx.h"

// Disable false alarm code analyze warning
#pragma warning (disable : 26165 26110)
namespace uvxx { namespace pplx { namespace details
{
    /// <summary>
    /// Spin lock to allow for locks to be used in global scope
    /// </summary>
    class _Spin_lock
    {
    public:

        _Spin_lock()
            : _M_lock(0)
        {
        }

        void lock()
        {
            if ( details::atomic_compare_exchange(_M_lock, 1l, 0l) != 0l )
            {
                do
                {
                    uvxx::pplx::details::platform::YieldExecution();

                } while ( details::atomic_compare_exchange(_M_lock, 1l, 0l) != 0l );
            }
        }

        void unlock()
        {
            // fence for release semantics
            details::atomic_exchange(_M_lock, 0l);
        }

    private:
        atomic_long _M_lock;
    };

    typedef uvxx::pplx::scoped_lock<_Spin_lock> _Scoped_spin_lock;
} // namespace details

    static struct _pplx_g_sched_t
    {
        typedef std::shared_ptr<uvxx::pplx::scheduler_interface> sched_ptr;

        _pplx_g_sched_t()
        {
            m_state = post_ctor;
        }

        ~_pplx_g_sched_t()
        {
            m_state = post_dtor;
        }

        uvxx::pplx::scheduler_interface* get_scheduler_ptr()
        {
            if (m_scheduler == nullptr)
            {
                get_scheduler();
            }

            uvxx::pplx::scheduler_interface* p = m_scheduler.get();

            return p;
        }

        sched_ptr get_scheduler()
        {
            switch (m_state)
            {
            case post_ctor:
                // This is the 99.9% case.

                if (!m_scheduler)
                {
                    uvxx::pplx::details::_Scoped_spin_lock lock(m_spinlock);
                    if (!m_scheduler)
                    {
                        m_scheduler = std::make_shared< uvxx::pplx::default_scheduler_t>();
                    }
                }

                return m_scheduler;
            default:
                // This case means the global m_scheduler is not available.
                // We spin off an individual scheduler instead.
                return std::make_shared< uvxx::pplx::default_scheduler_t>();
            }
        }

        void set_scheduler(const sched_ptr& scheduler)
        {
            if (m_state == pre_ctor || m_state == post_dtor) {
                throw invalid_operation("Scheduler cannot be initialized now");
            }

            uvxx::pplx::details::_Scoped_spin_lock lock(m_spinlock);

            if (m_scheduler != nullptr)
            {
                throw invalid_operation("Scheduler is already initialized");
            }

            m_scheduler = std::move(scheduler);
        }

        enum
        {
            pre_ctor = 0,
            post_ctor = 1,
            post_dtor = 2
        } m_state;

    private:
        uvxx::pplx::details::_Spin_lock m_spinlock;
        sched_ptr m_scheduler;
    } _pplx_g_sched;

    std::shared_ptr<uvxx::pplx::scheduler_interface> get_ambient_scheduler()
    {
        return _pplx_g_sched.get_scheduler();
    }

    uvxx::pplx::scheduler_interface* get_ambient_scheduler_ptr()
    {
        return _pplx_g_sched.get_scheduler_ptr();
    }

    void set_ambient_scheduler(const std::shared_ptr<uvxx::pplx::scheduler_interface>& _Scheduler)
    {
        _pplx_g_sched.set_scheduler(std::move(_Scheduler));
    }

}}

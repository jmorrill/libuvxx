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
* Parallel Patterns Library - Linux version
*
* For the latest on this and related APIs, please see http://casablanca.codeplex.com.
*
* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
****/
#if !defined(_MSC_VER) 
#undef __cplusplus
#define __cplusplus 201103L
#endif
#include <thread>
#include <functional>
#include "pplx/pplx.h"
#include "pplx/threadpool.h"

namespace uvxx { 
    namespace pplx { 
        namespace details { 
            namespace platform
            {
            #if !defined(_MSC_VER) 
                    long GetCurrentThreadId()
                    {
                        auto d = pthread_self();
                        return (long)d;
                    }
            #else
                    long __cdecl GetCurrentThreadId() 
                    { 
                        return (long)(::GetCurrentThreadId()); 
                    } 
            #endif

                    void YieldExecution()
                    {
                        std::this_thread::yield();
                    }
            }

    void linux_scheduler::schedule( TaskProc_t proc, void* param)
    {
        ::crossplat::threadpool::shared_instance().schedule(std::bind(proc, param));
    }

}}}// namespace uvxx

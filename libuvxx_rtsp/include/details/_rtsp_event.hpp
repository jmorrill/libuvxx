#pragma once
#include <functional>
#include "pplx/pplxtasks.h"

namespace uvxx { namespace rtsp { namespace details 
{
    class _rtsp_event
    {
    public:
        _rtsp_event(std::function<void()> continue_after)
        {

        }
        
    public:
        uvxx::pplx::task<void> then()
        {
            uvxx::pplx::task<void> t = uvxx::pplx::create_task([]
            {
            });

        }

    private:
    };
}}}
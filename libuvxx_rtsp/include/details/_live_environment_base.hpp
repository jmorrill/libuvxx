#pragma once
#include "details/_live_common.hpp"

namespace uvxx { namespace rtsp { namespace details 
{
    /* This class provide access to the live555 usage environment and 
       also keeps the reference alive until all live555 objects have
       been properly destroyed. */
    class _live_environment_base
    {
    protected:
        _live_environment_base();

    protected:
        _usage_environment_ptr live_usage_environment();

    private:
        _usage_environment_ptr _usage_environement;
    };
}}}
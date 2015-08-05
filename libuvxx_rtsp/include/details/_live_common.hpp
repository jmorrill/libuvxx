#pragma once
#include "BasicUsageEnvironment.hh"
#include <memory>

namespace uvxx { namespace rtsp { namespace details
{
    using _usage_environment_ptr = std::shared_ptr<UsageEnvironment>;

    _usage_environment_ptr _get_live_environment();
}}}


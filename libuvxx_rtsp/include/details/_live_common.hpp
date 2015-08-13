#pragma once
#include <memory>
#include "BasicUsageEnvironment.hh"

namespace uvxx { namespace rtsp { namespace details
{
    using _usage_environment_ptr = std::shared_ptr<UsageEnvironment>;

    using _usage_environment_weak_ptr = std::weak_ptr<UsageEnvironment>;

    _usage_environment_ptr _get_live_environment();
}}}


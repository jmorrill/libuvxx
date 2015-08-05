#include "details/_live_common.hpp"
#include "details/_uvxx_task_scheduler.hpp"

using namespace uvxx::rtsp::details;

thread_local _usage_environment_ptr _live_environment_;

_usage_environment_ptr uvxx::rtsp::details::_get_live_environment()
{
    if (_live_environment_)
    {
        return _live_environment_;
    }

    auto task_scheduler = _uvxx_task_scheduler::createNew();

    _live_environment_ = _usage_environment_ptr(BasicUsageEnvironment::createNew(*task_scheduler),
        /* deleter*/
        [](UsageEnvironment* environment)
    {
        auto& task_scheduler = environment->taskScheduler();

        delete &task_scheduler;

        if (!environment->reclaim())
        {
            assert(false);
        }
    });

    return _live_environment_;
}
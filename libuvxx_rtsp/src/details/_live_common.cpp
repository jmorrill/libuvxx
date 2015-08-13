#include "details/_live_common.hpp"
#include "details/_uvxx_task_scheduler.hpp"

using namespace uvxx::rtsp::details;

_usage_environment_ptr uvxx::rtsp::details::_get_live_environment()
{
    static thread_local _usage_environment_ptr _live_environment_;

    if (_live_environment_)
    {
        return _live_environment_;
    }

    auto scheduler = std::make_unique<_uvxx_task_scheduler>(1000000);

     _live_environment_ = _usage_environment_ptr(BasicUsageEnvironment::createNew(*scheduler),
    [](UsageEnvironment* environment)
    {
        auto& task_scheduler = environment->taskScheduler();

        delete &task_scheduler;

        if (!environment->reclaim())
        {
            assert(false);
        }
    });

    /* usage environment owns this pointer now,
       so don't delete me brah */
    scheduler.release();

    return _live_environment_;
}
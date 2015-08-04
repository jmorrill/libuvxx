#include "BasicUsageEnvironment.hh"

#include "details/_live_server_media_session.hpp"
#include "details/_server_media_session_impl.hpp"
#include "details/_uvxx_task_scheduler.hpp"

using namespace uvxx::rtsp::details;

_server_media_session_impl::_server_media_session_impl()
{
    _task_scheduler = _uvxx_task_scheduler::createNew();

    _usage_environment = _usage_environment_ptr(BasicUsageEnvironment::createNew(*_task_scheduler),
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

    __live_server_media_session = std::make_shared<_live_server_media_session>(_usage_environment);
}
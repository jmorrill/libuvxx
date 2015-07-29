#include "BasicUsageEnvironment.hh"

#include "details/_rtsp_server_impl.hpp"
#include "details/_live_rtsp_server.hpp"
#include "details/_uvxx_task_scheduler.hpp"

using namespace uvxx::rtsp::details;

_rtsp_server_impl::_rtsp_server_impl(uint16_t port) 
{
    _task_scheduler = _uvxx_task_scheduler::createNew();

    _usage_environment = _usage_environment_ptr(BasicUsageEnvironment::createNew(*_task_scheduler),
    /* deleter*/
    [](UsageEnvironment* env)
    {
        auto& task_scheduler = env->taskScheduler();

        delete &task_scheduler;

        if (!env->reclaim())
        {
            assert(false);
        }
    });

    _live_server = _live_rtsp_server_ptr(new _live_rtsp_server(_usage_environment, port),
    /* deleter */
    [](_live_rtsp_server* client)
    {
        Medium::close(client);
    });
}
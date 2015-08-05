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
        /* we need to keep the live env/task_scheduler alive until 
           all has been deleted by the liveMedia library */
        event_dispatcher::current_dispatcher().begin_invoke([=]
        {
            auto& task_scheduler = environment->taskScheduler();

            delete &task_scheduler;

            if (!environment->reclaim())
            {
                assert(false);
            }
        });
    });

    __live_server_media_session = _live_server_media_session_ptr(new _live_server_media_session(_usage_environment),
    [](_live_server_media_session* session)
    {
        if(!session->is_externally_owned())
        {
            delete session;
        }
    });

    __live_server_media_session->on_session_closed(std::bind(&_server_media_session_impl::on_session_closed, this));
}

void _server_media_session_impl::on_session_closed()
{
    __live_server_media_session = nullptr;
}
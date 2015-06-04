#pragma once
#include "uvxx.hpp"
#include "event_dispatcher_object.hpp"
#include "_live_rtsp_client.hpp"
#include "_uvxx_task_scheduler.h"
#include "BasicUsageEnvironment.hh"

#include <memory>

namespace uvxx { namespace uvxx_rtsp { namespace details 
{

    class _rtsp_client_impl : public uvxx::event_dispatcher_object
    {
    public:
        _rtsp_client_impl();

        uvxx::pplx::task<void> open(const std::string& url);

        ~_rtsp_client_impl()
        {
            bool success = _usage_environment->reclaim();
        }
    private:
        std::unique_ptr<_live_rtsp_client> _live_client;
        UsageEnvironment* _usage_environment;
        std::unique_ptr<_uvxx_task_scheduler> _task_scheduler;
    };
}}}
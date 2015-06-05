#pragma once
#include "uvxx.hpp"
#include "event_dispatcher_object.hpp"
#include "_live_rtsp_client.hpp"
#include "_uvxx_task_scheduler.h"
#include "_media_session.hpp"

#include "BasicUsageEnvironment.hh"

#include <memory>
#include <map>
#include <string>
namespace uvxx { namespace rtsp { namespace details 
{

    class _rtsp_client_impl : public uvxx::event_dispatcher_object
    {
    public:
        _rtsp_client_impl();

        uvxx::pplx::task<void> open(const std::string& url);

        ~_rtsp_client_impl();
    private:
        static void describe_callback(RTSPClient* rtspClient, int resultCode, char* resultString);

    private:
        uvxx::pplx::task_completion_event<int> _describe_event;

        _media_session _session;

        std::shared_ptr<_live_rtsp_client> _live_client;

        std::shared_ptr<UsageEnvironment> _usage_environment;

        _uvxx_task_scheduler* _task_scheduler;
    };
}}}
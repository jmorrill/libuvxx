#pragma once
#include "uvxx.hpp"
#include "event_dispatcher_object.hpp"
#include "_live_rtsp_client.hpp"
#include "_uvxx_task_scheduler.h"
#include "_media_session_impl.hpp"

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

        ~_rtsp_client_impl();

    public:
        uvxx::pplx::task<void> open(const std::string& url);

        uvxx::pplx::task<void> play();

        std::shared_ptr<_media_session> media_session_get();

    private:
        static void describe_callback(RTSPClient* live_rtsp_client, int result_code, char* result_string);

        static void setup_callback(RTSPClient* live_rtsp_client, int result_code, char* result_string);

        static void play_callback(RTSPClient* live_rtsp_client, int result_code, char* result_string);

    private:
        uvxx::pplx::task_completion_event<int> _describe_event;

        uvxx::pplx::task_completion_event<int> _setup_event;

        uvxx::pplx::task_completion_event<int> _play_event;

        std::shared_ptr<_media_session> _session;

        std::shared_ptr<_live_rtsp_client> _live_client;

        std::shared_ptr<UsageEnvironment> _usage_environment;

        _uvxx_task_scheduler* _task_scheduler;
    };
}}}
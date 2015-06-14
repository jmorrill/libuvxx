#pragma once
#include "uvxx.hpp"
#include "_live_rtsp_client.hpp"
#include "_uvxx_task_scheduler.h"
#include "_media_session_impl.hpp"
#include "_streaming_media_session_impl.hpp"
#include "streaming_media_session.hpp"
#include "_live_authenticator.hpp"
#include "BasicUsageEnvironment.hh"
#include "media_session.hpp"

#include <memory>
#include <map>
#include <string>

namespace uvxx { namespace rtsp
{
    class media_subsession;
}}

namespace uvxx { namespace rtsp { namespace details 
{
    using _usage_environment_ptr = std::shared_ptr<UsageEnvironment>;

    using _rtsp_client_impl_ptr = std::shared_ptr<_rtsp_client_impl>;

    class _rtsp_client_impl : public uvxx::event_dispatcher_object
    {
    public:
        _rtsp_client_impl();

        ~_rtsp_client_impl();

    public:
        uvxx::pplx::task<void> open(const std::string& url);

        uvxx::pplx::task<streaming_media_session> play(std::vector<media_subsession> subsessions);

        media_session media_session_get();

    private:
        uvxx::pplx::task<void> setup(const std::shared_ptr<std::vector<media_subsession>>& subsessions);

    private:
        static void describe_callback(RTSPClient* live_rtsp_client, int result_code, char* result_string);

        static void setup_callback(RTSPClient* live_rtsp_client, int result_code, char* result_string);

        static void play_callback(RTSPClient* live_rtsp_client, int result_code, char* result_string);

    private:
        uvxx::pplx::task_completion_event<int> _describe_event;

        uvxx::pplx::task_completion_event<int> _setup_event;

        uvxx::pplx::task_completion_event<int> _play_event;

        media_session _session;

        _live_rtsp_client_ptr _live_client;

        _usage_environment_ptr _usage_environment;

        _uvxx_task_scheduler* _task_scheduler;

        media_subsession _current_media_subsession_setup;

        streaming_media_session _streaming_session;

        _live_authenticator _authenticator;
    };

    using _rtsp_client_impl_ptr = std::shared_ptr<_rtsp_client_impl>;
}}}
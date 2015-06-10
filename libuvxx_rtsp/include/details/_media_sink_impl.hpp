#pragma once
#include "uvxx.hpp"
#include "event_dispatcher_object.hpp"
#include "_live_rtsp_client.hpp"
#include "_uvxx_task_scheduler.h"
#include "_media_session_impl.hpp"
#include "MediaSink.hh"
#include "media_session.hpp"
#include "_live_media_sink.hpp"
#include <memory>
#include <map>
#include <string>

namespace uvxx { namespace rtsp { namespace details 
{
    class _media_sink_impl : uvxx::event_dispatcher_object
    {
    public:
        _media_sink_impl(const _media_subsession_impl& media_subsession,
                         const std::shared_ptr<UsageEnvironment>& live_usage_environment);

    private:
        void on_frame(const _live_media_sink_frame& frame);

    private:
        _media_subsession_impl _media_subsession;

        std::shared_ptr<_live_media_sink> __live_media_sink;

        std::shared_ptr<UsageEnvironment> _live_usage_environment;
    };

}}}
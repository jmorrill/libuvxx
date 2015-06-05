#pragma once

#include "uvxx.hpp"
#include "event_dispatcher_object.hpp"
#include "_live_rtsp_client.hpp"
#include "_uvxx_task_scheduler.h"
#include "BasicUsageEnvironment.hh"
#include "MediaSession.hh"

#include <memory>
#include <string>

namespace uvxx { namespace rtsp { namespace details 
{
    class _media_subsession : public uvxx::event_dispatcher_object
    {
    public:
        _media_subsession(MediaSubsession* live_subsession);

        _media_subsession();

        _media_subsession(const _media_subsession&) = delete;

        _media_subsession& operator=(const _media_subsession&) = delete;

        _media_subsession(_media_subsession&& rhs);

        _media_subsession& operator=(_media_subsession&& rhs);

        bool initiate(int use_special_rtp_offset = -1);

        virtual ~_media_subsession();

    private:
        MediaSubsession* _live_subsession;
    };

    class _media_session : public uvxx::event_dispatcher_object
    {
    public:
        _media_session();

        _media_session(const _media_session&) = delete;

        _media_session& operator=(const _media_session&) = delete;

        _media_session(_media_session&& rhs);

        _media_session& operator=(_media_session&& rhs);

        void reset();

        void set_media_session(const std::shared_ptr<UsageEnvironment>& usage_environment, MediaSession* live_session);

        virtual ~_media_session();

    private:
        std::vector<_media_subsession> _subsessions;
        std::shared_ptr<UsageEnvironment> _usage_environment;
        MediaSession* _live_session;
    };
}}}
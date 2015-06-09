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
    using _usage_environment_ptr = std::shared_ptr<UsageEnvironment>;

    class _media_subsession_impl : public uvxx::event_dispatcher_object
    {
    public:
        _media_subsession_impl(MediaSubsession* live_subsession);

        _media_subsession_impl();

        _media_subsession_impl(const _media_subsession_impl&) = default;

        _media_subsession_impl& operator=(const _media_subsession_impl&) = default;

        _media_subsession_impl(_media_subsession_impl&& rhs);

        _media_subsession_impl& operator=(_media_subsession_impl&& rhs);

        virtual ~_media_subsession_impl();

    public:
        MediaSubsession* live_media_subsession_get() const;

        bool initiate(int use_special_rtp_offset = -1);

        std::string codec_name_get();

    private:
        MediaSubsession* _live_subsession;
    };

    using _media_subsession_impl_ptr = std::shared_ptr<_media_subsession_impl>;

    class _media_session_impl : public uvxx::event_dispatcher_object
    {
    public:
        _media_session_impl();

        _media_session_impl(const _media_session_impl&) = default;

        _media_session_impl& operator=(const _media_session_impl&) = default;

        _media_session_impl(_media_session_impl&& rhs);

        _media_session_impl& operator=(_media_session_impl&& rhs);

    public:
        void set_media_session(const _usage_environment_ptr& usage_environment, MediaSession* live_session);

        virtual ~_media_session_impl();

        const std::vector<_media_subsession_impl_ptr>& subsessions() const;   

        MediaSession* live_media_session_get() const;

    private:
        void reset();

    private:
        std::vector<_media_subsession_impl_ptr> _subsessions;

        _usage_environment_ptr _usage_environment;

        MediaSession* _live_session;
    };

    using _media_session_impl_ptr = std::shared_ptr<_media_session_impl>;
}}}
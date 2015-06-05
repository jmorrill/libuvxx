#pragma once

#include "uvxx.hpp"
#include "event_dispatcher_object.hpp"
#include "_live_rtsp_client.hpp"
#include "_uvxx_task_scheduler.h"
#include "BasicUsageEnvironment.hh"
#include "MediaSession.hh"

#include <memory>
#include <string>

namespace uvxx { namespace uvxx_rtsp { namespace details 
{
    class _media_subsession : public uvxx::event_dispatcher_object
    {
    public:
        _media_subsession(MediaSubsession* live_subsession)
        {
            _live_subsession = live_subsession;
        }

        _media_subsession() : _live_subsession(nullptr)
        {

        }

        _media_subsession(const _media_subsession&) = delete;

        _media_subsession& operator=(const _media_subsession&) = delete;

        _media_subsession(_media_subsession&& rhs)
        {
            _live_subsession = rhs._live_subsession;
            _live_subsession = nullptr;
        }

        _media_subsession& operator=(_media_subsession&& rhs)
        {
            return std::move(rhs);
        }

        bool initiate(int use_special_rtp_offset = -1)
        {
            return _live_subsession->initiate(use_special_rtp_offset);
        }

        virtual ~_media_subsession()
        {
        }
    private:
        MediaSubsession* _live_subsession;
    };

    class _media_session : public uvxx::event_dispatcher_object
    {
    public:
        _media_session() : _live_session(nullptr)
        {

        }

        _media_session(const _media_session&) = delete;

        _media_session& operator=(const _media_session&) = delete;

        _media_session(_media_session&& rhs)
        {
            _live_session = rhs._live_session;
            _live_session = nullptr;

            _subsessions = std::move(_subsessions);
        }

        _media_session& operator=(_media_session&& rhs)
        {
            return std::move(rhs);
        }

        void set_media_session(std::shared_ptr<UsageEnvironment> usage_environment, MediaSession* live_session)
        {
            _subsessions.clear();

            _live_session = live_session;
            _usage_environment = usage_environment;

            auto iter = std::make_unique<MediaSubsessionIterator>(*_live_session);
    
            while (auto live_subsession = iter->next())
            {
                if (!live_subsession)
                {
                    break;
                }

                _subsessions.emplace_back( _media_subsession(live_subsession));
            }
        }

        virtual ~_media_session()
        {
            if (_live_session)
            {
                Medium::close(_live_session);
            }
        }
    private:
        std::vector<_media_subsession> _subsessions;
        std::shared_ptr<UsageEnvironment> _usage_environment;
        MediaSession* _live_session;
    };
}}}
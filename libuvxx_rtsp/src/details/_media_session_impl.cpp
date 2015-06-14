#pragma once
#include "details/_media_session_impl.hpp"

using namespace uvxx::rtsp::details;

_media_subsession_impl::_media_subsession_impl(MediaSubsession* live_subsession)
{
    _live_subsession = live_subsession;
}

_media_subsession_impl::_media_subsession_impl() : _live_subsession(nullptr)
{

}

_media_subsession_impl::_media_subsession_impl(_media_subsession_impl&& rhs)
{
    _live_subsession = rhs._live_subsession;
    _live_subsession = nullptr;
}

_media_subsession_impl& _media_subsession_impl::operator=(_media_subsession_impl&& rhs)
{
    return std::move(rhs);
}

bool _media_subsession_impl::initiate(int use_special_rtp_offset /*= -1*/)
{
    return _live_subsession->initiate(use_special_rtp_offset);
}

_media_subsession_impl::~_media_subsession_impl()
{

}

std::string _media_subsession_impl::codec_name()
{
    return _live_subsession->codecName();
}

MediaSubsession* _media_subsession_impl::live_media_subsession() const
{
    return _live_subsession;
}

_media_session_impl::~_media_session_impl()
{
    if (_live_session)
    {
        Medium::close(_live_session);
    }
}

void _media_session_impl::live_media_session_set(const _usage_environment_ptr& usage_environment, MediaSession* live_session)
{
    reset();

    _live_session = live_session;

    _usage_environment = usage_environment;

    auto iter = std::make_unique<MediaSubsessionIterator>(*_live_session);

    while (auto live_subsession = iter->next())
    {
        if (!live_subsession)
        {
            break;
        }

        _subsessions.emplace_back(std::make_shared<_media_subsession_impl>(live_subsession));
    }
}

void _media_session_impl::reset()
{
    _subsessions.clear();
    _usage_environment = nullptr;
}

_media_session_impl& _media_session_impl::operator=(_media_session_impl&& rhs)
{
    return std::move(rhs);
}

_media_session_impl::_media_session_impl(_media_session_impl&& rhs)
{
    _live_session = rhs._live_session;
    _live_session = nullptr;

    _subsessions = std::move(_subsessions);
}

_media_session_impl::_media_session_impl() : _live_session(nullptr)
{

}

MediaSession* _media_session_impl::live_media_session() const
{
    return _live_session;
}

const std::vector<_media_subsession_impl_ptr>& _media_session_impl::subsessions() const
{
    return _subsessions;
}

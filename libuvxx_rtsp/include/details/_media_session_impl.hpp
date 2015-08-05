#pragma once
#include <memory>
#include <string>
#include <vector>

#include "_live_environment_base.hpp"

class MediaSubsession;

class MediaSession;

namespace uvxx { namespace rtsp { namespace details 
{
    class _media_subsession_impl : protected _live_environment_base
    {
    public:
        _media_subsession_impl(int stream_number, MediaSubsession* live_subsession);

        _media_subsession_impl();

        _media_subsession_impl(const _media_subsession_impl&) = delete;

        _media_subsession_impl& operator=(const _media_subsession_impl&) = delete;

        _media_subsession_impl(_media_subsession_impl&& rhs);

        _media_subsession_impl& operator=(_media_subsession_impl&& rhs);

        virtual ~_media_subsession_impl();

    public:
        MediaSubsession* live_media_subsession() const;

        bool initiate(int use_special_rtp_offset = -1);

        std::string codec_name() const;

        std::string medium_name() const;

        int stream_number() const;

        std::string get_attribute(const std::string& attribute_name) const;

        uint32_t rtp_timestamp_frequency() const;

        uint32_t channel_count() const;

    private:
        MediaSubsession* _live_subsession;

        int _stream_number;

        std::string _medium_name;
    };

    using _media_subsession_impl_ptr = std::shared_ptr<_media_subsession_impl>;

    class _media_session_impl : protected _live_environment_base
    {
    public:
        _media_session_impl();

        _media_session_impl(const _media_session_impl&) = delete;

        _media_session_impl& operator=(const _media_session_impl&) = delete;

        _media_session_impl(_media_session_impl&& rhs);

        _media_session_impl& operator=(_media_session_impl&& rhs);
        
    public:
        void live_media_session_set(MediaSession* live_session);

        virtual ~_media_session_impl();

    public:
        const std::vector<_media_subsession_impl_ptr>& subsessions() const;   

        MediaSession* live_media_session() const;

    private:
        void reset();

    private:
        std::vector<_media_subsession_impl_ptr> _subsessions;

        MediaSession* _live_session;
    };

    using _media_session_impl_ptr = std::shared_ptr<_media_session_impl>;
}}}
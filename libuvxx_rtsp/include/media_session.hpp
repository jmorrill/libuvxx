#pragma once
#include "event_dispatcher_object.hpp"

namespace uvxx { namespace rtsp { namespace details
{
    /* implementation forwards */

    class _media_subsession_impl;

    class _media_session_impl;

    class _rtsp_client_impl;

    class _streaming_media_session_impl;

    using _media_subsession_impl_ptr = std::shared_ptr<_media_subsession_impl>;

    using _media_session_impl_ptr = std::shared_ptr<_media_session_impl>;

    namespace media_framers
    {
        class _media_framer_base;
    }
}}}

namespace uvxx { namespace rtsp 
{
    class media_subsession : public event_dispatcher_object
    {
    public:
	    explicit media_subsession(const details::_media_subsession_impl_ptr& _media_subsession);

        media_subsession(const media_subsession&) = default;

        media_subsession& operator=(const media_subsession&) = default;

        media_subsession(media_subsession&& rhs);

        media_subsession& operator=(media_subsession&& rhs);

        bool operator=(std::nullptr_t rhs);

        bool operator==(std::nullptr_t rhs);

    public:
        std::string codec_name() const;

        std::string get_attribute(const std::string& attribute_name) const;

        int stream_number() const;

    private:
        media_subsession();

        std::shared_ptr<details::_media_subsession_impl> __media_subsession;

        friend std::allocator<media_subsession>;

        friend details::_rtsp_client_impl;

        friend details::_streaming_media_session_impl;

        friend details::media_framers::_media_framer_base;
    };

    class media_session : public event_dispatcher_object
    {
    public:
        media_session();

        virtual ~media_session();

	    explicit media_session(const details::_media_session_impl_ptr& _media_session);

        media_session(const media_session&) = default;

        media_session& operator=(const media_session&) = default;

        media_session(media_session&& dispatcher);

        media_session& operator=(media_session&& rhs);

    public:
        std::vector<media_subsession> subsessions() const;

        size_t subsession_count() const;
        
        media_subsession subsession(size_t index) const;

    private:
        details::_media_session_impl_ptr __media_session;

    };
}}
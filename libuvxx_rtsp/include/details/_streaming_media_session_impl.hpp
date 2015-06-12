#pragma once
#include "uvxx.hpp"
#include "media_session.hpp"
#include "MediaSession.hh"
#include <memory>
#include <vector>

namespace uvxx { namespace rtsp
{
    class media_subsession;

    namespace details
    {
        using _media_session_impl_ptr = std::shared_ptr<_media_session_impl>;
    }
}}

namespace uvxx { namespace rtsp { namespace details 
{
    class _streaming_media_session_impl
    {
    public:
        _streaming_media_session_impl(const _media_session_impl_ptr& session, const std::vector<media_subsession>& subsessions);

        _streaming_media_session_impl(const _streaming_media_session_impl& rhs) = delete;

        _streaming_media_session_impl& operator=(const _streaming_media_session_impl& rhs) = delete;

        virtual ~_streaming_media_session_impl();

    public:
        void on_frame_callback_set(std::function<bool()> callback);

    private:
        void continue_reading();

        static void on_rtcp_bye(void* client_data);

        static void on_after_getting_frame(void* client_data, unsigned packet_data_size, unsigned truncated_bytes, struct timeval presentation_time, unsigned duration_in_microseconds);
    
    private:
        std::vector<media_subsession> _subsessions;

        std::function<bool()> _on_frame_callback;

        std::vector<uint8_t> _buffer;

        _media_session_impl_ptr _session;
    };

    using _streaming_media_session_impl_ptr = std::shared_ptr<_streaming_media_session_impl>;
}}}
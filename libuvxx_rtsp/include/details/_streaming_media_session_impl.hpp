#pragma once
#include <memory>
#include <vector>

#include "rtsp_client.hpp"
#include "media_session.hpp"

namespace uvxx { namespace rtsp
{
    class media_subsession;

    class media_sample;

    namespace details
    {
        namespace media_framers
        {
            class _media_framer_base;
        }

        using _media_session_impl_ptr = std::shared_ptr<_media_session_impl>;
    }
}}

namespace uvxx { namespace rtsp { namespace details 
{
    class _streaming_media_session_impl
    {
    public:
        _streaming_media_session_impl(const media_session& session, std::vector<media_subsession> subsessions);

        _streaming_media_session_impl(const _streaming_media_session_impl& rhs) = delete;

        _streaming_media_session_impl& operator=(const _streaming_media_session_impl& rhs) = delete;

        bool operator=(std::nullptr_t rhs);

        bool operator==(std::nullptr_t rhs);

        virtual ~_streaming_media_session_impl();

    public:
        void read_stream_sample();

        void on_sample_set(read_sample_delegate callback);
	    
	    void on_stream_closed_set(stream_closed_delegate callback);

        stream_statistics stream_statistics_get(int stream_id) const;

    private:
        void close();

    private:
        std::vector<media_subsession> _subsessions;

        std::vector<std::shared_ptr<media_framers::_media_framer_base>> _media_framers;

        read_sample_delegate _on_sample_callback;

	    stream_closed_delegate _stream_closed_delegate;
	    
        media_session _session;
    };

    using _streaming_media_session_impl_ptr = std::shared_ptr<_streaming_media_session_impl>;
}}}
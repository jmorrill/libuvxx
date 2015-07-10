#pragma once
#include "uvxx.hpp"
#include "media_session.hpp"
#include "MediaSession.hh"
#include "_media_sample_impl.hpp"
#include "media_sample.hpp"
#include "details/media_framers/_media_framer_base.h"
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
        _streaming_media_session_impl(const media_session& session, std::vector<media_subsession> subsessions);

        _streaming_media_session_impl(const _streaming_media_session_impl& rhs) = delete;

        _streaming_media_session_impl& operator=(const _streaming_media_session_impl& rhs) = delete;

        virtual ~_streaming_media_session_impl();

    public:
        void on_frame_callback_set(std::function<bool(const media_sample&)> callback);

    private:
        std::vector<media_subsession> _subsessions;

        std::vector<std::shared_ptr<media_framers::_media_framer_base>> _media_framers;

        std::function<bool(const media_sample&)> _on_frame_callback;

        media_session _session;
    };

    using _streaming_media_session_impl_ptr = std::shared_ptr<_streaming_media_session_impl>;
}}}
#pragma once
#include <memory>
#include <chrono>
#include <string>

namespace uvxx { namespace rtsp { namespace details
{
    class _media_sample_impl;

    class _streaming_media_session_impl;
}}}

namespace uvxx { namespace rtsp
{
    class media_sample
    {
    public:
        media_sample();

        media_sample(const media_sample&) = default;

        media_sample& operator=(const media_sample&) = default;

    public:
        const int stream_number() const;

        const size_t size() const;

        const uint8_t* data() const;

        const std::chrono::microseconds presentation_time() const;

        bool is_complete_sample() const;

        bool is_truncated() const;

        const std::string codec_name() const;

    private:
        std::shared_ptr<details::_media_sample_impl> __media_sample_impl;

        friend details::_streaming_media_session_impl;
    };
}}
    
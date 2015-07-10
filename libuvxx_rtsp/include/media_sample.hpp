#pragma once
#include <memory>
#include <chrono>
#include <string>
#include "io/memory_buffer.hpp"

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

        void size_set(size_t size) const;

        const size_t capacity() const;

        void capacity_set(size_t size) const;

        const uint8_t* data() const;

        const std::chrono::microseconds presentation_time() const;

        void presentation_time_set(std::chrono::microseconds presentation_time) const;

        bool is_complete_sample() const;

        bool is_truncated() const;

        void is_truncated_set(bool truncated) const;

        void codec_name_set(const std::string& codec_name);

        const std::string codec_name() const;

        void set_attribute(const std::string& attribute_name, const uvxx::io::memory_buffer& buffer) const;

        uvxx::io::memory_buffer get_attribute(const std::string& attribute_name) const;

    private:
        std::shared_ptr<details::_media_sample_impl> __media_sample_impl;

        friend details::_streaming_media_session_impl;
    };
}}
    
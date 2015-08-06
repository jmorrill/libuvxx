#pragma once
#include <memory>
#include <chrono>
#include <string>
#include <cstring>

#include "io/memory_buffer.hpp"
#include "media_attributes.hpp"

namespace uvxx { namespace rtsp { namespace details
{
    class _media_sample_impl;

    using _media_sample_impl_ptr = std::shared_ptr<_media_sample_impl>;
}}}

namespace uvxx { namespace rtsp
{
    class media_sample : public media_attributes
    {
    public:
        media_sample();

        explicit media_sample(const details::_media_sample_impl_ptr& impl);

        media_sample(const media_sample&) = default;

        media_sample& operator=(const media_sample&) = default;

        media_sample(media_sample&& rhs);

        media_sample& operator=(media_sample&& rhs);

    public:
        int stream_number() const;

        void stream_number_set(int stream_number);

        size_t size() const;

        void size_set(size_t size);

        size_t capacity() const;

        void capacity_set(size_t size);

        const uint8_t* data() const;

        std::chrono::microseconds presentation_time() const;

        void presentation_time_set(std::chrono::microseconds presentation_time);
       
        bool is_truncated() const;

        void is_truncated_set(bool truncated);

        void codec_name_set(const std::string& codec_name);

        std::string codec_name() const;

    private:
        std::shared_ptr<details::_media_sample_impl> __media_sample_impl;
    };
}}
    
#pragma once
#include <memory>
#include <chrono>
#include <string>

namespace uvxx { namespace rtsp { namespace details 
{
    class _media_sample_impl
    {
    public:
        _media_sample_impl();

        _media_sample_impl(const _media_sample_impl&) = delete;

        _media_sample_impl& operator=(const _media_sample_impl&) = delete;

    public:
        const size_t size() const;

        void size_set(size_t size);

        const uint8_t* data() const;

        const std::chrono::microseconds presentation_time() const;
       
        void presentation_time_set(std::chrono::microseconds presentation_time);

        bool is_complete_sample() const;

        void is_complete_sample_set(bool complete_sample);

        bool is_truncated() const;

        void is_truncated_set(bool truncated);

        const std::string codec_name() const;

        void codec_name_set(const std::string& codec_name);
    private:
        size_t _size;

        std::chrono::microseconds _presentation_time;

        bool _is_complete_sample;

        bool _is_truncated;

        std::string _codec_name;
    };
}}}
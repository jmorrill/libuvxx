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

    public:
        const size_t size() const;

        const uint8_t* data() const;

        const std::chrono::microseconds presentation_time() const;
       
        bool complete_sample() const;

        bool truncated() const;

        const std::string codec_name() const;
    };
}}}
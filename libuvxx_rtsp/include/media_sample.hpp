#pragma once
#include <memory>
#include <chrono>
#include <string>

#include "media_attributes.hpp"
#include "io/memory_buffer.hpp"

namespace uvxx { namespace rtsp { namespace details
{
    class _media_sample_impl;
}}}

namespace uvxx { namespace rtsp
{
    class media_sample
    {
    public:
        media_sample();

        media_sample(const media_sample&) = default;

        media_sample& operator=(const media_sample&) = default;

		media_sample(media_sample&& rhs);

		media_sample& operator=(media_sample&& rhs);

    public:
        int stream_number() const;

        void stream_number_set(int stream_number);

        size_t size() const;

        void size_set(size_t size) const;

        size_t capacity() const;

        void capacity_set(size_t size) const;

        const uint8_t* data() const;

        std::chrono::microseconds presentation_time() const;

        void presentation_time_set(std::chrono::microseconds presentation_time) const;
       
        bool is_truncated() const;

        void is_truncated_set(bool truncated) const;

        void codec_name_set(const std::string& codec_name);

        std::string codec_name() const;

        void clear_attributes();

        void attribute_blob_set(const std::string& attribute_name, const io::memory_buffer& buffer) const;

        io::memory_buffer attribute_blob_get(const std::string& attribute_name) const;

        template<typename T>
        void attribute_set(const std::string& attribute_name, T value)
        {
            auto buffer = attribute_blob_get(attribute_name);

            if (buffer)
            {
                if (buffer.length_get() == sizeof(T) &&
                    !memcmp(buffer.data(), static_cast<void*>(&value), sizeof(T)))
                {
                    return;
                }
            }

            if (!buffer || buffer.length_get() != sizeof(T))
            {
                buffer = io::memory_buffer(sizeof(T));
            }

            memcpy(buffer.data(), &value, sizeof(T));

            attribute_blob_set(attribute_name, buffer);
        }

        template<typename T>
        T attribute_get(const std::string& attribute_name) const
        {
            auto buffer = attribute_blob_get(attribute_name);

            if (!buffer)
            {
                return T();
            }

            T value = (*reinterpret_cast<T*>(static_cast<uint8_t*>(buffer)));

            return value;
        }

    private:
        std::shared_ptr<details::_media_sample_impl> __media_sample_impl;
    };
}}
    
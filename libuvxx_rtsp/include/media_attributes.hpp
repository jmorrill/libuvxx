#pragma once

#pragma once
#include <memory>
#include <string>
#include <cstring>

#include "io/memory_buffer.hpp"

namespace uvxx { namespace rtsp { namespace details
{
    class _media_attributes_impl;

    using _media_attributes_impl_ptr = std::shared_ptr<_media_attributes_impl>;
}}}

namespace uvxx { namespace rtsp
{
    class media_attributes
    {
    public:
        media_attributes();

        explicit media_attributes(const details::_media_attributes_impl_ptr& attributes);
    public:

        void clear_attributes();

        void attribute_blob_set(const std::string& attribute_name, const io::memory_buffer& buffer);

        io::memory_buffer attribute_blob_get(const std::string& attribute_name) const;

        template<typename T>
        void attribute_set(const std::string& attribute_name, const T& value)
        {
            auto buffer = attribute_blob_get(attribute_name);

            if (buffer)
            {
                if (buffer.length_get() == sizeof(T) &&
                    !memcmp(buffer.data(), static_cast<void*>(const_cast<T*>(std::addressof(value))), sizeof(T)))
                {
                    return;
                }
            }

            if (!buffer || buffer.length_get() != sizeof(T))
            {
                buffer = io::memory_buffer(sizeof(T));
            }

            memcpy(buffer.data(), std::addressof(value), sizeof(T));

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

            T value = (*reinterpret_cast<T*>(buffer.data()));

            return value;
        }

        void copy_attributes_from(const media_attributes& attributes);

    private:
        details::_media_attributes_impl_ptr ___media_attributes_impl;
    };

    template<>
    inline void media_attributes::attribute_set<>(const std::string& attribute_name, const std::string& value)
    {
        auto buffer = attribute_blob_get(attribute_name);

        if (buffer)
        {
            if (buffer.length_get() == value.size() &&
                !memcmp(buffer.data(), static_cast<void*>(const_cast<char*>(value.c_str())), value.size()))
            {
                return;
            }
        }

        if (!buffer || buffer.length_get() != value.size())
        {
            buffer = io::memory_buffer(value.size());
        }

        memcpy(buffer.data(), value.c_str(), value.size());

        attribute_blob_set(attribute_name, buffer);
    }

    template<>
    inline std::string media_attributes::attribute_get(const std::string& attribute_name) const
    {
        auto buffer = attribute_blob_get(attribute_name);

        if (!buffer)
        {
            return std::string();
        }

        auto value = std::string((reinterpret_cast<char*>(buffer.data())));

        return value;
    }
}}
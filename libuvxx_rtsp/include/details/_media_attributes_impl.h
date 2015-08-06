#pragma once
#include <memory>
#include <chrono>
#include <string>
#include <vector>
#include <unordered_map>

#include "io/memory_buffer.hpp"

namespace uvxx { namespace rtsp { namespace details 
{
    class _media_attributes_impl;

    using _media_attributes_impl_ptr = std::shared_ptr<_media_attributes_impl>;

    class _media_attributes_impl
    {
    public:
        _media_attributes_impl();

        _media_attributes_impl(const _media_attributes_impl&) = delete;

        _media_attributes_impl& operator=(const _media_attributes_impl&) = delete;

        virtual ~_media_attributes_impl() = default;
    public:
        void clear_attributes();

        void attribute_set(const std::string& attribute_name, const io::memory_buffer& buffer);

        io::memory_buffer attribute_get(const std::string& attribute_name);

        void copy_attributes_from(const _media_attributes_impl_ptr& attributes);

    private:
        std::unordered_map<std::string, io::memory_buffer> _attribute_map;
    };
}}}
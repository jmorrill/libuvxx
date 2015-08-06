#pragma once
#include <memory>
#include <chrono>
#include <string>
#include <vector>
#include "media_descriptor.hpp"
#include "media_attributes.hpp"

namespace uvxx { namespace rtsp { namespace details 
{

    class _stream_descriptor_impl
    {
    public:
        _stream_descriptor_impl();

        virtual ~_stream_descriptor_impl() = default;

    public:
        int stream_id() const;

        void stream_id_set(int id);

        const std::string codec_name() const;

        void codec_name_set(const std::string& codec_name);

        media_attributes attributes() const;

    private:
        int _stream_id;

        std::string _codec_name;

        media_attributes _attributes;
    };

    class _media_descriptor_impl
    {
    public:
        _media_descriptor_impl();

    public:
        void add_stream_from_attributes(int stream_id, const std::string& codec_name, const media_attributes& attributes);

        const std::vector<stream_descriptor> get_streams() const;

    private:
        std::vector<stream_descriptor> _descriptors;
    };
}}}
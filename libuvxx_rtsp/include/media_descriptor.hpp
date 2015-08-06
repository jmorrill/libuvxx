#pragma once
#include "event_dispatcher_object.hpp"

namespace uvxx { namespace rtsp { namespace details
{
    class _media_descriptor_impl;

    class _stream_descriptor_impl;

    using _stream_descriptor_impl_ptr = std::shared_ptr<_stream_descriptor_impl>;

    using _media_descriptor_impl_ptr = std::shared_ptr<_media_descriptor_impl>;
}}}

namespace uvxx { namespace rtsp 
{
    class media_attributes;

    class stream_descriptor
    {
    public:
        stream_descriptor();

        virtual ~stream_descriptor() = default;

    public:
        int stream_id() const;

        void stream_id_set(int id);

        const std::string codec_name() const;

        void codec_name_set(const std::string& codec_name);

        media_attributes attributes() const;

    private:
        details::_stream_descriptor_impl_ptr __stream_descriptor_impl;
    };

    class media_descriptor
    {
    public:
        media_descriptor();

    public:
        void add_stream_from_attributes(int stream_id, const std::string& codec_name, const media_attributes& attributes);

        const std::vector<stream_descriptor> get_streams() const;

    private:
        details::_media_descriptor_impl_ptr __media_descriptor_impl;
    };
}}
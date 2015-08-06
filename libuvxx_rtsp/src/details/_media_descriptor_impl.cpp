#include "details/_media_descriptor_impl.hpp"

using namespace uvxx::rtsp;
using namespace uvxx::rtsp::details;

_media_descriptor_impl::_media_descriptor_impl()
{
}

void _media_descriptor_impl::add_stream_from_attributes(int stream_id, const std::string& codec_name, const media_attributes& attributes)
{
    stream_descriptor stream;

    stream.stream_id_set(stream_id);

    stream.codec_name_set(codec_name);

    stream.attributes().copy_attributes_from(attributes);

    _descriptors.emplace_back(stream);
}

const std::vector<stream_descriptor> _media_descriptor_impl::get_streams() const
{
    return _descriptors;
}

_stream_descriptor_impl::_stream_descriptor_impl() : _stream_id(0)
{
}

int _stream_descriptor_impl::stream_id() const
{
    return _stream_id;
}

void _stream_descriptor_impl::stream_id_set(int id)
{
    _stream_id = id;
}

const std::string _stream_descriptor_impl::codec_name() const
{
    return _codec_name;
}

media_attributes _stream_descriptor_impl::attributes() const
{
    return _attributes;
}

void _stream_descriptor_impl::codec_name_set(const std::string& codec_name)
{
    _codec_name = codec_name;
}
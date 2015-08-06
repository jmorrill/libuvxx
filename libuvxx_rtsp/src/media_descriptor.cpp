#include "media_descriptor.hpp"
#include "media_attributes.hpp"
#include "details/_media_descriptor_impl.hpp"

using namespace uvxx::rtsp;
using namespace uvxx::rtsp::details;

media_descriptor::media_descriptor() :
    __media_descriptor_impl(std::make_shared<_media_descriptor_impl>())
{
}

void media_descriptor::add_stream_from_attributes(int stream_id, const std::string& codec_name, const media_attributes& attributes)
{
    __media_descriptor_impl->add_stream_from_attributes(stream_id, codec_name, attributes);
}

const std::vector<stream_descriptor> media_descriptor::get_streams() const
{
    return __media_descriptor_impl->get_streams();
}

stream_descriptor::stream_descriptor() :
    __stream_descriptor_impl(std::make_shared<_stream_descriptor_impl>())
{
}

int stream_descriptor::stream_id() const
{
    return __stream_descriptor_impl->stream_id();
}

void stream_descriptor::stream_id_set(int id)
{
     __stream_descriptor_impl->stream_id_set(id);
}

const std::string stream_descriptor::codec_name() const
{
    return __stream_descriptor_impl->codec_name();
}

void stream_descriptor::codec_name_set(const std::string& codec_name)
{
    __stream_descriptor_impl->codec_name_set(codec_name);
}

media_attributes stream_descriptor::attributes() const
{
    return __stream_descriptor_impl->attributes();
}
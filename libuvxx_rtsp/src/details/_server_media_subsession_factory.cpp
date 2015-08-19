#include "sample_attributes.hpp"

#include "details/_audio_media_subsession.hpp"
#include "details/_server_media_subsession_factory.hpp"
#include "details/_h264_media_subsession.hpp"
#include "details/_mpeg4es_media_subsession.hpp"

using namespace uvxx::rtsp;
using namespace uvxx::rtsp::details;
using namespace uvxx::rtsp::sample_attributes;

_live_server_media_subsession* uvxx::rtsp::details::_create_server_media_subsession(const stream_descriptor& descriptor)
{
    if(descriptor.codec_name() == "H264" || descriptor.codec_name() == "h264")
    {
        return new _h264_media_subsession(descriptor.stream_id(), descriptor.attributes());
    }
    else if (descriptor.codec_name() == "MP4V-ES" || descriptor.codec_name() == "mp4v-es")
    {
        return new _mpeg4es_media_subsession(descriptor.stream_id(), descriptor.attributes());
    }

    auto attributes = descriptor.attributes();

    auto major_type = attributes.attribute_get<sample_major_type>(ATTRIBUTE_SAMPLE_MAJOR_TYPE);

    if(major_type == sample_major_type::audio)
    {
        return new _audio_media_subsession(descriptor.stream_id(), descriptor.codec_name(), descriptor.attributes());
    }

    return nullptr;
}
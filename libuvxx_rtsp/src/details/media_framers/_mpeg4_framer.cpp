#include "H264VideoRTPSource.hh"
#include "MPEG4LATMAudioRTPSource.hh"
#include "sample_attributes.hpp"
#include "details/media_framers/_mpeg4_framer.hpp"
#include "details/media_framers/mpeg4es/_mpeg4es_parser.hpp"

using namespace uvxx::rtsp;
using namespace uvxx::rtsp::sample_attributes;
using namespace uvxx::rtsp::details::media_framers;
using namespace uvxx::rtsp::details::media_framers::mpeg4es;

static const char * CONFIG_ATTRIBUTE = "config";

_mpeg4_framer::_mpeg4_framer(const media_subsession& subsession) :
    _media_framer_base(subsession),
    _has_received_key_frame(false),
    _video_dimensions({ 0, 0 })
{
    use_rtp_marker_for_pts_set(true);

    auto config_data = subsession.get_attribute(CONFIG_ATTRIBUTE);

    uint32_t config_count = 0;

    std::unique_ptr<uint8_t[]> unsafe_ptr( parseGeneralConfigStr(config_data.c_str(), config_count));

    _config_data = std::vector<uint8_t>(unsafe_ptr.get(), unsafe_ptr.get() + config_count);

    auto sample = working_sample();

    parse_sample_data(sample);

    sample.attribute_blob_set(ATTRIBUTE_MPEG4_CONFIG_DATA, _config_data);
}

_mpeg4_framer::~_mpeg4_framer()
{

}

void _mpeg4_framer::sample_receieved(bool packet_marker_bit)
{
    auto media_sample = working_sample();

    parse_sample_data(media_sample);

    _media_framer_base::sample_receieved(packet_marker_bit);
}

void _mpeg4_framer::parse_sample_data(uvxx::rtsp::media_sample& sample )
{
    mpeg4es_parser parser(sample.data(), sample.size());

    auto width = parser.video_width();

    auto height = parser.video_height();

    auto is_key_frame = parser.is_key_frame();

    if (width && height)
    {
        if (_video_dimensions.width != width || _video_dimensions.height != height)
        {
            _video_dimensions.width = width;

            _video_dimensions.height = height;

            sample.attribute_set(ATTRIBUTE_VIDEO_DIMENSIONS, _video_dimensions);
        }
    }

    sample.attribute_set(ATTRIBUTE_VIDEO_KEYFRAME, is_key_frame);
}

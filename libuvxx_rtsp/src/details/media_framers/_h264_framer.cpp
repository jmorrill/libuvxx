#include "details/media_framers/_h264_framer.hpp"
#include "details/media_framers/h26x/sps_parser.hpp"

const int NAL_SEQUENCE_PARAMETER_SET = 7;
const int NAL_PICTURE_PARAMETER_SET = 8;

using namespace uvxx::rtsp::details::media_framers::h26x;

uvxx::rtsp::details::media_framers::_h264_framer::_h264_framer(const media_subsession& subsession, int stream_number) :
    _media_framer_base(subsession, stream_number),
    _has_received_key_frame(false)
{
    auto prop_parameter_sets = subsession.get_attribute("sprop-parameter-sets");

    uint32_t parameter_set_count = 0;

    std::unique_ptr<SPropRecord[]> parameter_records(parseSPropParameterSets(prop_parameter_sets.c_str(), parameter_set_count));

    for (uint32_t i = 0; i < parameter_set_count; i++)
    {
        uint8_t* record_bytes = parameter_records.get()[i].sPropBytes;

        int record_len = parameter_records.get()[i].sPropLength;

        if (record_len)
        {
            int nal_type = record_bytes[0] & 0x1F;

            if (nal_type == NAL_SEQUENCE_PARAMETER_SET)
            {
                _sequence_parameter_set = std::vector<uint8_t>(record_bytes, record_bytes + record_len);

                sps_parser parser(_sequence_parameter_set);

                _video_dimensions.width = parser.video_width();

                _video_dimensions.height = parser.video_height();
            }
            else if (nal_type == NAL_PICTURE_PARAMETER_SET)
            {
                _picture_parameter_set = std::vector<uint8_t>(record_bytes, record_bytes + record_len);
            }
        }
    }
}

uvxx::rtsp::details::media_framers::_h264_framer::~_h264_framer()
{

}

void uvxx::rtsp::details::media_framers::_h264_framer::sample_receieved()
{
    auto& media_sample = sample();

    int nal_type = media_sample.data()[0] & 0x1F;

    bool full_sample_ready = false;

    if (nal_type == NAL_SEQUENCE_PARAMETER_SET)
    {
        _sequence_parameter_set = std::vector<uint8_t>(media_sample.data(), media_sample.data() + media_sample.size());

        sps_parser parser(_sequence_parameter_set);

        _video_dimensions.width = parser.video_width();

        _video_dimensions.height = parser.video_height();
    }
    else if (nal_type == NAL_PICTURE_PARAMETER_SET)
    {
        _picture_parameter_set = std::vector<uint8_t>(media_sample.data(), media_sample.data() + media_sample.size());
    }
    else if (nal_type <= 5)
    {
        if (nal_type == 5)
        {
            _has_received_key_frame = true;
        }

        full_sample_ready = true;
    }
    else
    {
        printf("undefined nal %d\n", nal_type);
    }

    if (!full_sample_ready)
    {
        _media_framer_base::continue_reading();
    }
    else
    {
        media_sample.attribute_value_set("media_sample_attributes.video.dimensions", _video_dimensions);

        auto x = media_sample.attribute_value_get<video_dimensions>("media_sample_attributes.video.dimensions");

        media_sample.attribute_blob_set("media_sample_attributes.h264.sps", _sequence_parameter_set);

        media_sample.attribute_blob_set("media_sample_attributes.h264.pps", _picture_parameter_set);

        _media_framer_base::sample_receieved();
    }
}

#include "details/media_framers/_h264_framer.hpp"

const int NAL_SEQUENCE_PARAMETER_SET = 7;
const int NAL_PICTURE_PARAMETER_SET = 8;

uvxx::rtsp::details::media_framers::_h264_framer::_h264_framer(const media_subsession& subsession, int stream_number) :
    _media_framer_base(subsession, stream_number)
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

    _media_framer_base::sample_receieved();
}

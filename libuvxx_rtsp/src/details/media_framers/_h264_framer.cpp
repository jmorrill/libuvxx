#include "details/media_framers/_h264_framer.hpp"
#include "details/media_framers/h26x/sps_parser.hpp"

using namespace uvxx::rtsp::details::media_framers::h26x;

static const char * RTSP_SPROP_PARAMETER_SET_ATTRIBUTE = "sprop-parameter-sets";

static const uint8_t NAL_TYPE_BIT_MASK = 0x1F;

enum class h26x_nal_type
{
    undefined = 0,
    slice = 1,
    slice_dpa = 2,
    slice_dpb = 3,
    slice_dpc = 4,
    slice_idr = 5,
    sei = 6,
    sequence_parameter_set = 7,
    picture_parameter_set = 8,
    au_delimiter = 9,
    end_sequence = 10,
    end_stream = 11,
    filler_data = 12,
    sps_extension = 13,
    nalu_prefix = 14,
    sps_subset = 15,
    aux_slice = 19,
    slice_extension = 20
};


static void set_to_vector_if_unequal(std::vector<uint8_t>& buffer, const uvxx::rtsp::media_sample& sample)
{
    bool are_equal = true;

    while (true)
    {
        if (buffer.size() != sample.size())
        {
            are_equal = false;

            break;
        }

        auto sample_data = sample.data();

        auto buffer_data = buffer.data();

        for (size_t i = 0; i < sample.size(); i++)
        {
            if (sample_data[i] != buffer[i])
            {
                are_equal = false;

                break;
            }
        }

        break;
    }

    if (are_equal)
    {
        return;
    }

    buffer = std::vector<uint8_t>(sample.data(), sample.data() + sample.size());
}

uvxx::rtsp::details::media_framers::_h264_framer::_h264_framer(const media_subsession& subsession) :
    _media_framer_base(subsession),
    _has_received_key_frame(false)
{
    auto prop_parameter_sets = subsession.get_attribute(RTSP_SPROP_PARAMETER_SET_ATTRIBUTE);

    uint32_t parameter_set_count = 0;

    std::unique_ptr<SPropRecord[]> parameter_records(parseSPropParameterSets(prop_parameter_sets.c_str(), parameter_set_count));

    for (uint32_t i = 0; i < parameter_set_count; i++)
    {
        uint8_t* record_bytes = parameter_records.get()[i].sPropBytes;

        int record_len = parameter_records.get()[i].sPropLength;

        if (record_len)
        {
            h26x_nal_type nal_type = static_cast<h26x_nal_type>(record_bytes[0] & NAL_TYPE_BIT_MASK);

            if (nal_type == h26x_nal_type::sequence_parameter_set)
            {
                _sequence_parameter_set = std::vector<uint8_t>(record_bytes, record_bytes + record_len);

                initialize_sequence_parameter_set();
            }
            else if (nal_type == h26x_nal_type::picture_parameter_set)
            {
                _picture_parameter_set = std::vector<uint8_t>(record_bytes, record_bytes + record_len);
            }
        }
    }

    auto& media_sample = working_sample();

    media_sample.attribute_set(media_sample_attributes::SAMPLE_MAJOR_TYPE, media_sample_attributes::media_sample_majortype::video);
}

uvxx::rtsp::details::media_framers::_h264_framer::~_h264_framer()
{

}

void uvxx::rtsp::details::media_framers::_h264_framer::sample_receieved(bool packet_marker_bit)
{
    auto& media_sample = working_sample();

    h26x_nal_type nal_type = static_cast<h26x_nal_type>(media_sample.data()[0] & NAL_TYPE_BIT_MASK);

    bool full_sample_ready = false;

    bool key_frame = false;

    if (nal_type == h26x_nal_type::sequence_parameter_set)
    {
        set_to_vector_if_unequal(_sequence_parameter_set, media_sample);

        initialize_sequence_parameter_set();
    }
    else if (nal_type == h26x_nal_type::picture_parameter_set)
    {
        set_to_vector_if_unequal(_picture_parameter_set, media_sample);
    }
    else if (nal_type <= h26x_nal_type::slice_idr)
    {
        if (nal_type == h26x_nal_type::slice_idr)
        {
            _has_received_key_frame = true;

            key_frame = true;
        }

        if (packet_marker_bit)
        {
            full_sample_ready = true;
        }
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
        media_sample.attribute_set(media_sample_attributes::VIDEO_KEYFRAME, key_frame);

        media_sample.attribute_set(media_sample_attributes::VIDEO_DIMENSIONS, _video_dimensions);

        media_sample.attribute_blob_set(media_sample_attributes::H26X_SEQUENCE_PARAMETER_SET, _sequence_parameter_set);

        media_sample.attribute_blob_set(media_sample_attributes::H26X_PICTURE_PARAMETER_SET, _picture_parameter_set);

        _media_framer_base::sample_receieved(packet_marker_bit);
    }
}

void uvxx::rtsp::details::media_framers::_h264_framer::initialize_sequence_parameter_set()
{
    sps_parser parser(_sequence_parameter_set);

    if (parser.video_width() && parser.video_height())
    {
        _video_dimensions.width = parser.video_width();

        _video_dimensions.height = parser.video_height();
    }
}

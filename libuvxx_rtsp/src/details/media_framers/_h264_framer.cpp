#include "details/media_framers/_h264_framer.hpp"
#include "details/media_framers/h26x/sps_parser.hpp"

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
            h26x_nal_type nal_type = static_cast<h26x_nal_type>(record_bytes[0] & 0x1F);

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
}

uvxx::rtsp::details::media_framers::_h264_framer::~_h264_framer()
{

}

void uvxx::rtsp::details::media_framers::_h264_framer::sample_receieved()
{
    auto& media_sample = sample();

    h26x_nal_type nal_type = static_cast<h26x_nal_type>(media_sample.data()[0] & 0x1F);

    bool full_sample_ready = false;

    if (nal_type == h26x_nal_type::sequence_parameter_set)
    {
        _sequence_parameter_set = std::vector<uint8_t>(media_sample.data(), media_sample.data() + media_sample.size());

        initialize_sequence_parameter_set();
    }
    else if (nal_type == h26x_nal_type::picture_parameter_set)
    {
        _picture_parameter_set = std::vector<uint8_t>(media_sample.data(), media_sample.data() + media_sample.size());
    }
    else if (nal_type <= h26x_nal_type::slice_idr)
    {
        if (nal_type == h26x_nal_type::slice_idr)
        {
            _has_received_key_frame = true;
        }

        bool sample_complete = media_sample.is_complete_sample();

        if (sample_complete)
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
        media_sample.attribute_value_set("media_sample.video.dimensions", _video_dimensions);

        auto x = media_sample.attribute_value_get<video_dimensions>("media_sample.video.dimensions");

        media_sample.attribute_blob_set("media_sample.h264.sps", _sequence_parameter_set);

        media_sample.attribute_blob_set("media_sample.h264.pps", _picture_parameter_set);

        printf("%d complete\n", media_sample.is_complete_sample());

        _media_framer_base::sample_receieved();
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

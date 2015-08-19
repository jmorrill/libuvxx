#include "H264VideoRTPSource.hh"
#include "MPEG4LATMAudioRTPSource.hh"
#include "details/media_framers/_mpeg4_framer.hpp"

using namespace uvxx::rtsp;
using namespace uvxx::rtsp::sample_attributes;
using namespace uvxx::rtsp::details::media_framers;

static const char * CONFIG_ATTRIBUTE = "config";

static bool set_to_vector_if_unequal(std::vector<uint8_t>& buffer, const media_sample& sample)
{
    bool are_equal = true;

    for (; ;)
    {
        if (buffer.size() != sample.size())
        {
            are_equal = false;

            break;
        }

        auto sample_data = sample.data();

        auto buffer_data = buffer.data();

        if (memcmp(sample_data, buffer_data, sample.size()))
        {
            are_equal = false;
        }

        break;
    }

    if (are_equal)
    {
        return false;
    }

    buffer = std::vector<uint8_t>(sample.data(), sample.data() + sample.size());

    return true;
}

static const uint32_t VIDEO_OBJECT_PLANE = 0x000001B6;


class mpeg4_parser
{
public:
    explicit mpeg4_parser(const std::vector<uint8_t>& data)
    {
        const size_t MIN_DATA_SIZE = 88;

        size_t buffer_size = data.size() * 8 /* convert to bits */;

        if (!buffer_size || buffer_size < MIN_DATA_SIZE)
        {
            return;
        }

        _buffer = data.data();

        _buffer_length = data.size();

        _current_bit = 0;

        try
        {
            parse_layer();
        }
        catch (std::out_of_range&)
        {

        }
    }

    int video_width() const
    {
        return _video_width;
    }

    int video_height() const
    {
        return _video_height;
    }

private:
    unsigned int read_bit()
    {
        if (!(_current_bit <= _buffer_length * 8))
        {
            throw std::out_of_range("parser does not have enough bits to parse");
        }

        int index = _current_bit / 8;

        int offset = _current_bit % 8 + 1;

        _current_bit++;

        return (_buffer[index] >> (8 - offset)) & 0x01;
    }

    unsigned int read_bits(int n)
    {
        int r = 0;

        int i;

        for (i = 0; i < n; i++)
        {
            r |= (read_bit() << (n - i - 1));
        }

        return r;
    }

    
    void parse_layer()
    {

    top:

        uint32_t start_code = (_buffer[0 + (_current_bit / 8)] << 24) | (_buffer[1 + (_current_bit / 8)] << 16) | (_buffer[2 + (_current_bit / 8)] << 8) | _buffer[3 + (_current_bit / 8)];

        /* move bit pointer */
        uint32_t bytes = read_bits(32);

        if (start_code == 0x000001B5)
        {
            bool is_visual_object_identifier = read_bit();

            if (is_visual_object_identifier)
            {
                uint8_t visual_object_verid = read_bits(4);
                uint8_t visual_object_priority = read_bits(3);
            }

            uint8_t visual_object_type = read_bits(4);

            if (visual_object_type == 1 || visual_object_type == 2)
            {
                uint8_t video_signal_type = read_bit();

                if (video_signal_type)
                {
                    uint8_t video_format = read_bits(3);

                    uint8_t video_range = read_bits(1);

                    uint8_t color_description = read_bit();

                    if (color_description)
                    {
                        uint8_t color_primaries = read_bits(8);

                        uint8_t transfer_characteristics = read_bits(8);

                        uint8_t matrix_coefficients = read_bits(8);
                    }
                }
            }

            /* skip user data todo */
            /* hacked byte alignment todo */
            read_bits(2);

            if (visual_object_type == 1)
            {
                if (read_bits(24) != 0x000001)
                {
                    return;
                }

                uint8_t video_object_start_code = read_bits(8);

                if (video_object_start_code < 0 || video_object_start_code > 0x1f)
                {
                    return;
                }

                if (read_bits(24) != 0x000001)
                {
                    return;
                }

                uint8_t video_object_layer_start_code = read_bits(8);

                if (video_object_layer_start_code < 0x20 || video_object_layer_start_code > 0x2f)
                {
                    return;
                }

                uint8_t random_accessible_vol = read_bit();

                uint8_t video_object_type_indication = read_bits(8);

                if (video_object_type_indication == 0x12)
                {
                    return;
                }

                uint8_t is_object_layer_identifier = read_bit();

                uint8_t video_object_layer_verid = 0;

                if (is_object_layer_identifier)
                {
                    video_object_layer_verid = read_bits(4);

                    uint8_t video_object_layer_priority = read_bits(3);
                }

                uint8_t aspect_ratio_info = read_bits(4);

                uint8_t parx, pary;

                switch (aspect_ratio_info)
                {
                    case 1: parx = 1; pary = 1; break;
                    case 2: parx = 12; pary = 11; break;
                    case 3: parx = 10; pary = 11; break;
                    case 4: parx = 16; pary = 11; break;
                    case 5: parx = 40; pary = 33; break;
                    case 15: parx = read_bits(8); pary = read_bits(8); break;
                }

                uint8_t vol_control_parameters = read_bit();

                if (vol_control_parameters)
                {   
                    uint8_t chroma_format = read_bits(2);

                    uint8_t low_delay = read_bit();

                    uint8_t vbv_parameters = read_bit();

                    if (vbv_parameters)
                    {
                        uint16_t first_half_bit_rate = read_bits(15);

                        if (!read_bit()) return;

                        uint16_t latter_half_bit_rate = read_bits(15);

                        if (!read_bit()) return;

                        uint16_t first_half_vbv_buffer_size = read_bits(15);

                        if (!read_bit()) return;
                    
                        uint8_t latter_half_vbv_buffer_size = read_bits(3);

                        uint16_t first_half_vbv_occupancy = read_bits(11);

                        if (!read_bit()) return;

                        uint16_t latter_half_vbv_occupancy = read_bits(15);

                        if (!read_bit()) return;
                    }
                }

                uint8_t video_object_layer_shape = read_bits(2);

                if (video_object_layer_shape == 3 && video_object_layer_verid != 1)
                {
                    uint8_t video_object_layer_shape_extension = read_bits(4);
                }

                read_bit();

                uint16_t vop_time_increment_resolution = read_bits(16);

                read_bit();

                uint8_t fixed_vop_rate = read_bit();

                if (fixed_vop_rate)
                {
                    int32_t bits = 0;

                    for (int16_t i = vop_time_increment_resolution; i; i /= 2)
                    {
                        bits++;
                    }

                    int16_t fixed_vop_time_increment = read_bits(bits);

                    if (fixed_vop_time_increment)
                    {
                        auto atpf = 10000000i64 * fixed_vop_time_increment / vop_time_increment_resolution;
                    }
                }

                if (video_object_layer_shape != 2)
                {
                    if (video_object_layer_shape == 0)
                    {
                        read_bit();

                        uint16_t width = read_bits(13);

                        read_bit();

                        uint16_t height = read_bits(13);

                        read_bit();
                    }
                }
            }
        }
        else if (start_code == 0x000001B0)
        {
            read_bits(8);

            goto top;
        }
    }
private:
    const uint8_t * _buffer;

    size_t _buffer_length;

    unsigned int _current_bit;

    int _video_width;

    int _video_height;
};

_mpeg4_framer::_mpeg4_framer(const media_subsession& subsession) :
    _media_framer_base(subsession),
    _has_received_key_frame(false)
{
    use_rtp_marker_for_pts_set(true);

    auto config_data = subsession.get_attribute(CONFIG_ATTRIBUTE);

    uint32_t config_count = 0;

    auto unsafe_ptr = parseGeneralConfigStr(config_data.c_str(), config_count);

    _config_data = std::vector<uint8_t>(unsafe_ptr, unsafe_ptr + config_count);

    mpeg4_parser parser(_config_data);

    delete[] unsafe_ptr;
}

_mpeg4_framer::~_mpeg4_framer()
{

}

void _mpeg4_framer::sample_receieved(bool packet_marker_bit)
{
    auto media_sample = working_sample();

    bool full_sample_ready = false;

    bool key_frame = false;

    mpeg4_parser parser(std::vector<uint8_t>(media_sample.data(), media_sample.data() + media_sample.size()));
    if (!full_sample_ready)
    {
        continue_reading();
    }
    else
    {
        media_sample.attribute_set(ATTRIBUTE_VIDEO_KEYFRAME, key_frame);

        media_sample.attribute_set(ATTRIBUTE_VIDEO_DIMENSIONS, _video_dimensions);

        _media_framer_base::sample_receieved(packet_marker_bit);
    }
}

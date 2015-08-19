#include "H264VideoRTPSource.hh"
#include "MPEG4LATMAudioRTPSource.hh"
#include "details/media_framers/_mpeg4_framer.hpp"
#include "sample_attributes.hpp"

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
    explicit mpeg4_parser(const uint8_t* data, size_t data_size) :
        _is_key_frame(false),
        _current_bit(0)
    {
        if (data_size == 0)
        {
            return;
        }

        _buffer = data;

        _buffer_length = data_size;

        try
        {
            parse_layer();
        }
        catch (std::out_of_range&)
        {

        }
    }

    int32_t video_width() const
    {
        return _video_width;
    }

    int32_t video_height() const
    {
        return _video_height;
    }

    bool is_key_frame()
    {
        return _is_key_frame;
    }

private:
    uint8_t read_bit()
    {
        if (!(_current_bit <= _buffer_length * 8))
        {
            throw std::out_of_range("parser does not have enough bits to parse");
        }

        uint32_t index = _current_bit / 8;

        uint32_t offset = _current_bit % 8 + 1;

        _current_bit++;

        return (_buffer[index] >> (8 - offset)) & 0x01;
    }

    uint32_t read_bits(uint32_t n)
    {
        uint32_t r = 0;

        uint32_t i;

        for (i = 0; i < n; i++)
        {
            r |= (read_bit() << (n - i - 1));
        }

        return r;
    }
    
    uint32_t read_next_start_code()
    {
        uint32_t start_code = 0;

        size_t offset = 0;

        for (; ;)
        {
            offset = _current_bit / 8;

            if (!(_buffer[0 + (offset)] == 0 && _buffer[1 + (offset)] == 0 && _buffer[2 + (offset)] == 1))
            {
                _current_bit += 8;

                if (!(_current_bit <= _buffer_length * 8))
                {
                    return 0;
                }

                continue;
            }

            break;
        }

        start_code = (_buffer[0 + (offset)] << 24) |
                     (_buffer[1 + (offset)] << 16) |
                     (_buffer[2 + (offset)] << 8)  | _buffer[3 + (offset)];

        /* move bit pointer */
        uint32_t bytes = read_bits(32);

        return start_code;
    }

    
    void byte_align()
    {
        uint32_t alignment = 8;

        _current_bit += alignment - 1;

        _current_bit -= _current_bit % alignment;
    }

    void parse_layer()
    {
        for (;;)
        {
            auto start_code = read_next_start_code();

            if (start_code == 0)
            {
                return;
            }


            if (start_code == 0x00001b5)
            {
                bool is_visual_object_identifier = static_cast<bool>(read_bit());

                if (is_visual_object_identifier)
                {
                    uint8_t visual_object_verid = static_cast<uint8_t>(read_bits(4));

                    uint8_t visual_object_priority = static_cast<uint8_t>(read_bits(3));
                }

                uint8_t visual_object_type = static_cast<uint8_t>(read_bits(4));

                if (visual_object_type == 1 || visual_object_type == 2)
                {
                    uint8_t video_signal_type = static_cast<uint8_t>(read_bit());

                    if (video_signal_type)
                    {
                        uint8_t video_format = static_cast<uint8_t>(read_bits(3));

                        uint8_t video_range = static_cast<uint8_t>(read_bits(1));

                        uint8_t color_description = static_cast<uint8_t>(read_bit());

                        if (color_description)
                        {
                            uint8_t color_primaries = static_cast<uint8_t>(read_bits(8));

                            uint8_t transfer_characteristics = static_cast<uint8_t>(read_bits(8));

                            uint8_t matrix_coefficients = static_cast<uint8_t>(read_bits(8));
                        }
                    }
                }

                /* skip user data todo */
                byte_align();

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

                        case 15: 
                            parx = read_bits(8); 

                            pary = read_bits(8); 
                        break;
                    }

                    auto vol_control_parameters = static_cast<uint8_t>(read_bit());

                    if (vol_control_parameters)
                    {
                        auto chroma_format = static_cast<uint8_t>(read_bits(2));

                        auto low_delay = static_cast<uint8_t>(read_bit());

                        auto vbv_parameters = static_cast<uint8_t>(read_bit());

                        if (vbv_parameters)
                        {
                            auto first_half_bit_rate = static_cast<uint16_t>(read_bits(15));

                            if (!read_bit())
                            {
                                continue;
                            }

                            auto latter_half_bit_rate = static_cast<uint16_t>(read_bits(15));

                            if (!read_bit()) 
                            {
                                continue;
                            }

                            auto first_half_vbv_buffer_size = static_cast<uint16_t>(read_bits(15));

                            if (!read_bit())
                            {
                                continue;
                            }

                            auto latter_half_vbv_buffer_size = static_cast<uint8_t>(read_bits(3));

                            auto first_half_vbv_occupancy = static_cast<uint16_t>(read_bits(11));

                            if (!read_bit())
                            {
                                continue;
                            }

                            auto latter_half_vbv_occupancy = static_cast<uint16_t>(read_bits(15));

                            if (!read_bit())
                            {
                                continue;
                            }
                        }
                    }

                    uint8_t video_object_layer_shape = static_cast<uint8_t>(read_bits(2));

                    if (video_object_layer_shape == 3 && video_object_layer_verid != 1)
                    {
                        uint8_t video_object_layer_shape_extension = static_cast<uint8_t>(read_bits(4));
                    }

                    read_bit();

                    auto vop_time_increment_resolution = static_cast<uint16_t>(read_bits(16));

                    read_bit();

                    uint8_t fixed_vop_rate = static_cast<uint16_t>(read_bit());

                    if (fixed_vop_rate)
                    {
                        int32_t bits = 0;

                        for (int16_t i = vop_time_increment_resolution; i; i /= 2)
                        {
                            bits++;
                        }

                        int16_t fixed_vop_time_increment = static_cast<uint16_t>(read_bits(bits));

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

                            _video_width = read_bits(13);

                            read_bit();

                            _video_height = read_bits(13);

                            read_bit();
                        }
                    }
                }
            }
            else if(start_code == 0x000001b0)
            {
                continue;
            }
            else if(start_code == 0x000001b3)
            {
                continue;
            }
            else if (start_code == 0x000001b6)
            {
                _is_key_frame = !static_cast<bool>(read_bits(2));
            }
        }
    }
private:
    const uint8_t * _buffer;

    size_t _buffer_length;

    uint32_t _current_bit;

    int32_t _video_width;

    int32_t _video_height;

    bool _is_key_frame;
};

_mpeg4_framer::_mpeg4_framer(const media_subsession& subsession) :
    _media_framer_base(subsession),
    _has_received_key_frame(false),
    _video_dimensions({ 0, 0 })
{
    use_rtp_marker_for_pts_set(true);

    auto config_data = subsession.get_attribute(CONFIG_ATTRIBUTE);

    uint32_t config_count = 0;
    
    auto unsafe_ptr = parseGeneralConfigStr(config_data.c_str(), config_count);

    _config_data = std::vector<uint8_t>(unsafe_ptr, unsafe_ptr + config_count);

    delete[] unsafe_ptr;

    auto sample = working_sample();

    parse_sample_data(sample);
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

void _mpeg4_framer::parse_sample_data(uvxx::rtsp::media_sample& sample)
{
    bool key_frame = false;

    mpeg4_parser parser(sample.data(), sample.size());

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

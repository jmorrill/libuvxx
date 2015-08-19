#include <stdexcept>
#include "details/media_framers/mpeg4es/_mpeg4es_parser.hpp"

using namespace uvxx::rtsp::details::media_framers::mpeg4es;

static const uint32_t VIDEO_OBJECT_SEQUENCE_START = 0x000001B0;
static const uint32_t VISUAL_OBJECT_SEQUENCE_END  = 0x000001B1;
static const uint32_t VIDEO_SEQUENCE_HEADER_START = 0x000001B3;
static const uint32_t VIDEO_OBJECT_START          = 0x000001B5;
static const uint32_t VIDEO_OBJECT_PLANE          = 0x000001B6;

#pragma warning(push, 3)

mpeg4es_parser::mpeg4es_parser(const uint8_t * data, size_t data_size) :
    _buffer(data),
    _buffer_length(data_size),
    _current_bit(0),
    _is_key_frame(false)
{
    if (!_buffer || !_buffer_length)
    {
        return;
    }

    try
    {
        parse_layer();
    }
    catch (std::out_of_range&)
    {

    }
}

int32_t mpeg4es_parser::video_width() const
{
    return _video_width;
}

int32_t mpeg4es_parser::video_height() const
{
    return _video_height;
}

bool mpeg4es_parser::is_key_frame() const
{
    return _is_key_frame;
}

uint8_t mpeg4es_parser::read_bit()
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

uint32_t mpeg4es_parser::read_bits(uint32_t n)
{
    uint32_t r = 0;

    uint32_t i;

    for (i = 0; i < n; i++)
    {
        r |= (read_bit() << (n - i - 1));
    }

    return r;
}

uint32_t mpeg4es_parser::read_next_start_code()
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
                 (_buffer[2 + (offset)] << 8) | _buffer[3 + (offset)];

    /* move bit pointer */
    read_bits(32);

    return start_code;
}

void mpeg4es_parser::byte_align()
{
    uint32_t alignment = 8;

    _current_bit += alignment - 1;

    _current_bit -= _current_bit % alignment;
}

void mpeg4es_parser::parse_layer()
{
    for (;;)
    {
        auto start_code = read_next_start_code();

        if (!start_code)
        {
            return;
        }

        if (start_code == VIDEO_OBJECT_START)
        {
            bool is_visual_object_identifier = static_cast<uint8_t>(read_bit()) != 0;

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

                uint8_t video_object_start_code = static_cast<uint8_t>(read_bits(8));

                if (video_object_start_code < 0 || video_object_start_code > 0x1f)
                {
                    return;
                }

                if (read_bits(24) != 0x000001)
                {
                    return;
                }

                uint8_t video_object_layer_start_code = static_cast<uint8_t>(read_bits(8));

                if (video_object_layer_start_code < 0x20 || video_object_layer_start_code > 0x2f)
                {
                    return;
                }

                uint8_t random_accessible_vol = static_cast<uint8_t>(read_bit());

                uint8_t video_object_type_indication = static_cast<uint8_t>(read_bits(8));

                if (video_object_type_indication == 0x12)
                {
                    return;
                }

                uint8_t is_object_layer_identifier = static_cast<uint8_t>(read_bit());

                uint8_t video_object_layer_verid = 0;

                if (is_object_layer_identifier)
                {
                    video_object_layer_verid = static_cast<uint8_t>(read_bits(4));

                    uint8_t video_object_layer_priority = static_cast<uint8_t>(read_bits(3));
                }

                uint8_t aspect_ratio_info = static_cast<uint8_t>(read_bits(4));

                uint8_t parx, pary;

                switch (aspect_ratio_info)
                {
                case 1: parx = 1; pary = 1; break;

                case 2: parx = 12; pary = 11; break;

                case 3: parx = 10; pary = 11; break;

                case 4: parx = 16; pary = 11; break;

                case 5: parx = 40; pary = 33; break;

                case 15:
                    parx = static_cast<uint8_t>(read_bits(8));

                    pary = static_cast<uint8_t>(read_bits(8));
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
                        auto atpf = 10000000ul * fixed_vop_time_increment / vop_time_increment_resolution;
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
        else if (start_code == VIDEO_OBJECT_SEQUENCE_START)
        {
            continue;
        }
        else if (start_code == VIDEO_SEQUENCE_HEADER_START)
        {
            continue;
        }
        else if (start_code == VIDEO_OBJECT_PLANE)
        {
            _is_key_frame = !static_cast<uint8_t>(read_bits(2));
        }
    }
}
#pragma warning(pop)
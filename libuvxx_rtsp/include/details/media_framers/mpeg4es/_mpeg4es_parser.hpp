#pragma once
#include "sample_attributes.hpp"

namespace uvxx { namespace rtsp { namespace details { namespace media_framers { namespace mpeg4es 
{
    class mpeg4es_parser
    {
    public:
        explicit mpeg4es_parser(const uint8_t* data, size_t data_size);

        int32_t video_width() const;

        int32_t video_height() const;

        bool is_key_frame() const;

    private:
        uint8_t read_bit();

        uint32_t read_bits(uint32_t n);

        uint32_t read_next_start_code();

        void byte_align();

        void parse_layer();

    private:
        const uint8_t * _buffer;

        size_t _buffer_length;

        uint32_t _current_bit;

        int32_t _video_width;

        int32_t _video_height;

        bool _is_key_frame;
    };
}}}}}
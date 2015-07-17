#pragma once
#include <vector>
#include <stdint.h>

namespace uvxx { namespace rtsp { namespace details { namespace media_framers { namespace h26x
{
    class sps_parser
    {
        public:
            sps_parser(const std::vector<uint8_t>& sequence_parameter_set);

            int video_width() const;

            int video_height() const;

        private:
            unsigned int read_bit();

            unsigned int read_bits(int n);

            unsigned int read_exponential_golomb_code();

            unsigned int read_se();

            void parse_sps();

        private:
            const unsigned char * _buffer;

            unsigned short _buffer_length;

            int _current_bit;

            int _video_width;
        
            int _video_height;
        };
}}}}}
#pragma once
#include <vector>
#include "H264VideoRTPSource.hh"

#include "_media_framer_base.hpp"

namespace uvxx { namespace rtsp { namespace details { namespace media_framers
{
    class _h264_framer : public _media_framer_base
    {
    public:
        _h264_framer(const media_subsession& subsession);

        virtual ~_h264_framer();
    
    protected:
        virtual void sample_receieved(bool packet_marker_bit);

    private:
        void initialize_sequence_parameter_set();

    private:
        std::vector<uint8_t> _sequence_parameter_set;

        std::vector<uint8_t> _picture_parameter_set;

        bool _has_received_key_frame;

        media_attributes::video_dimensions _video_dimensions;
    };
}}}}
#pragma once
#include <vector>
#include "_media_framer_base.h"
#include "H264VideoRTPSource.hh"

namespace uvxx { namespace rtsp { namespace details { namespace media_framers
{
    struct video_dimensions
    {
        int width; int height;
    };

    class _h264_framer : public _media_framer_base
    {
    public:
        _h264_framer(const media_subsession& subsession, int stream_number);

        virtual ~_h264_framer();
    
    protected:
        virtual void sample_receieved();

    private:
        std::vector<uint8_t> _sequence_parameter_set;

        std::vector<uint8_t> _picture_parameter_set;

        bool _has_received_key_frame;

        video_dimensions _video_dimensions;
    };
}}}}
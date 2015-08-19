#pragma once
#include <vector>

#include "_media_framer_base.hpp"
#include "sample_attributes.hpp"

namespace uvxx { namespace rtsp { namespace details { namespace media_framers
{
    class _mpeg4_framer : public _media_framer_base
    {
    public:
        explicit _mpeg4_framer(const media_subsession& subsession);

        virtual ~_mpeg4_framer();
    
    protected:
        virtual void sample_receieved(bool packet_marker_bit) override;

    private:
        void initialize_sequence_parameter_set();

        void parse_sample_data(uvxx::rtsp::media_sample& sample);

    private:
        std::vector<uint8_t> _config_data;

        bool _has_received_key_frame;

        sample_attributes::video_dimensions _video_dimensions;
    };
}}}}
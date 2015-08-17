#pragma once
#include "media_session.hpp"
#include "_media_framer_base.hpp"

namespace uvxx { namespace rtsp { namespace details { namespace media_framers
{
    class _g726_audio_framer : public _media_framer_base
    {
    public:
        explicit _g726_audio_framer(const media_subsession& subsession);

        virtual ~_g726_audio_framer();

    protected:
        virtual void sample_receieved(bool packet_marker_bit) override;
    };

}}}}
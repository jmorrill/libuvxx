#include "details/media_framers/_g726_audio_framer.hpp"
#include "sample_attributes.hpp"

using namespace uvxx::rtsp;
using namespace uvxx::rtsp::sample_attributes;
using namespace uvxx::rtsp::details::media_framers;

static const uint32_t BITS_PER_SAMPLE = 16;

_g726_audio_framer::_g726_audio_framer(const media_subsession& subsession) : _media_framer_base(subsession)
{
    auto sample = _media_framer_base::working_sample();

    auto codec_name = _subsession.codec_name();

    sample.attribute_set(ATTRIBUTE_AUDIO_BITS_PER_SAMPLE, BITS_PER_SAMPLE);

    sample.attribute_set(ATTRIBUTE_AUDIO_CHANNEL_COUNT, _subsession.channel_count());

    sample.attribute_set(ATTRIBUTE_AUDIO_SAMPLES_PER_SECOND, _subsession.rtp_timestamp_frequency());
}

_g726_audio_framer::~_g726_audio_framer()
{
}

void _g726_audio_framer::sample_receieved(bool packet_marker_bit)
{
    _media_framer_base::sample_receieved(packet_marker_bit);
}
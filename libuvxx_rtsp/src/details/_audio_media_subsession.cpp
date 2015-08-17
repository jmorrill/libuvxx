#include "SimpleRTPSink.hh"

#include "media_attributes.hpp"
#include "details/_audio_media_subsession.hpp"
#include "details/_audio_framed_source.hpp"

using namespace uvxx::rtsp;
using namespace uvxx::rtsp::details;
using namespace uvxx::rtsp::sample_attributes;

_audio_media_subsession::_audio_media_subsession(int stream_id, const std::string& codec_name, const media_attributes& attributes) : 
    _live_server_media_subsession(stream_id, attributes),
    _codec_name(codec_name)
{

}

FramedSource* _audio_media_subsession::createNewStreamSource(unsigned client_session_id, unsigned& estimated_kbps)
{
    _source = std::shared_ptr<_audio_framed_source>(new _audio_framed_source(stream_id(), client_session_id), [](_live_framed_source*)
    {
        /* todo add logic later in case live555 doesn't free */
    });

    estimated_kbps = 64; /* kbps, estimate */

    notify_framed_source(client_session_id);

    return _source.get();
}

RTPSink* _audio_media_subsession::createNewRTPSink(Groupsock* rtp_groupsock, unsigned char rtp_payload_type_if_dynamic, FramedSource* input_source)
{
    auto attributes = get_attributes();

    auto channel_count = attributes.attribute_get<uint32_t>(ATTRIBUTE_AUDIO_CHANNEL_COUNT);

    auto sampling_frequency = attributes.attribute_get<uint32_t>(ATTRIBUTE_AUDIO_SAMPLES_PER_SECOND);

    return SimpleRTPSink::createNew(envir(), rtp_groupsock, 8, sampling_frequency, "audio", _codec_name.c_str(), channel_count);
}
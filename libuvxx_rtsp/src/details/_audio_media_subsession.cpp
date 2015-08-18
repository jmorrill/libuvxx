#include "SimpleRTPSink.hh"

#include "media_attributes.hpp"
#include "details/_audio_media_subsession.hpp"
#include "details/_audio_framed_source.hpp"
#include <MPEG1or2AudioRTPSink.hh>

using namespace uvxx::rtsp;
using namespace uvxx::rtsp::details;
using namespace uvxx::rtsp::sample_attributes;

uint8_t get_payload_type(uint8_t default_payload_format_code, const std::string& codec_name, const media_attributes& attributes)
{
    auto channel_count = attributes.attribute_get<uint32_t>(ATTRIBUTE_AUDIO_CHANNEL_COUNT);

    auto sampling_frequency = attributes.attribute_get<uint32_t>(ATTRIBUTE_AUDIO_SAMPLES_PER_SECOND);

    if(codec_name == "PCMU")
    {
        if(sampling_frequency == 80000 && channel_count == 1)
        {
            return 0;
        }

        return default_payload_format_code;
    }

    if(codec_name == "L16" && sampling_frequency == 44100)
    {
        if(channel_count == 1)
        {
            return 11;
        }
        else if(channel_count == 2)
        {
            return 10;
        }

        return default_payload_format_code;
    }

    if(codec_name == "PCMA")
    {
        if(sampling_frequency == 8000 && channel_count == 1)
        {
            return 8;
        }
    }

    if(codec_name == "DVI4")
    {
        if(channel_count == 1)
        {
            if(sampling_frequency == 8000)
            {
                return 5;
            }
            else if(sampling_frequency == 16000)
            {
                return 6;
            }
            else if(sampling_frequency == 11025)
            {
                return 16;
            }
            else if(sampling_frequency == 22050)
            {
                return 17;
            }
        }

        return default_payload_format_code;
    }

    return default_payload_format_code;
}

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

RTPSink* _audio_media_subsession::createNewRTPSink(Groupsock* rtp_groupsock, unsigned char rtp_payload_type_if_dynamic, FramedSource* /*input_source*/)
{
    auto attributes = get_attributes();

    auto channel_count = attributes.attribute_get<uint32_t>(ATTRIBUTE_AUDIO_CHANNEL_COUNT);

    auto sampling_frequency = attributes.attribute_get<uint32_t>(ATTRIBUTE_AUDIO_SAMPLES_PER_SECOND);

    auto payload_format_code = get_payload_type(rtp_payload_type_if_dynamic, _codec_name, attributes);

    if(_codec_name == "MPA")
    {
        return MPEG1or2AudioRTPSink::createNew(envir(), rtp_groupsock);
    }

    return SimpleRTPSink::createNew(envir(), rtp_groupsock, payload_format_code, sampling_frequency, "audio", _codec_name.c_str(), channel_count);
}
#pragma once
#include "_live_server_media_subsession.hpp"

namespace uvxx { namespace rtsp { namespace details 
{
    class _audio_media_subsession : public _live_server_media_subsession
    {
    public:
        _audio_media_subsession(int stream_id, const std::string& codec_name, const media_attributes& attributes);

    private:
        virtual FramedSource* createNewStreamSource(unsigned client_session_id, unsigned& estimated_kbps) override;

        virtual RTPSink* createNewRTPSink(Groupsock* rtp_groupsock, unsigned char rtp_payload_type_if_dynamic, FramedSource* input_source) override;
    
    private:
        std::string _codec_name;
    };
}}}
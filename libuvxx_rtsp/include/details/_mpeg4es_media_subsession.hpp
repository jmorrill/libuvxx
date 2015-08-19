#pragma once
#include "event_dispatcher_object.hpp"
#include "event_dispatcher_frame.hpp"
#include "_live_server_media_subsession.hpp"

namespace uvxx { namespace rtsp {
    class media_attributes;
}}

namespace uvxx { namespace rtsp { namespace details 
{
    class _live_framed_source;

    class _mpeg4es_media_subsession : public _live_server_media_subsession
    {
    public:
        explicit _mpeg4es_media_subsession(int stream_id, const media_attributes&);

        _mpeg4es_media_subsession(const _mpeg4es_media_subsession&) = delete;

        const _mpeg4es_media_subsession& operator=(const _mpeg4es_media_subsession&) = delete;

        virtual ~_mpeg4es_media_subsession();

    protected:
        virtual FramedSource* createNewStreamSource(unsigned client_session_id, unsigned& estimated_kbps) override;

        virtual RTPSink* createNewRTPSink(Groupsock* rtp_groupsock, unsigned char rtp_payload_type_if_dynamic, FramedSource* input_source) override;

        virtual void closeStreamSource(FramedSource *inputSource) override;

        static void after_playing_dummy_callback(void* client_data);
       
    private:

    };
 }}}
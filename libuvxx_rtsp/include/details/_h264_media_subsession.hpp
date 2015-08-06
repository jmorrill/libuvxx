#pragma once
#include "OnDemandServerMediaSubsession.hh"

#include "event_dispatcher_object.hpp"

namespace uvxx { namespace rtsp { namespace details 
{
    using framed_source_factory_delegate = std::function<FramedSource*(int stream_id, unsigned client_session_id)>;

    class _h264_media_subsession : public OnDemandServerMediaSubsession, public event_dispatcher_object
    {
    public:
        explicit _h264_media_subsession(int stream_id);

        _h264_media_subsession(const _h264_media_subsession&) = delete;

        const _h264_media_subsession& operator=(const _h264_media_subsession&) = delete;

        virtual ~_h264_media_subsession();

        void source_factory_create_set(framed_source_factory_delegate factory);

    protected:
        void setDoneFlag() { fDoneFlag = ~0; }

    protected:
        virtual char const* getAuxSDPLine(RTPSink* rtp_sink, FramedSource* input_source) override;

        virtual FramedSource* createNewStreamSource(unsigned client_session_id, unsigned& estimated_kbps) override;

        virtual RTPSink* createNewRTPSink(Groupsock* rtp_groupsock, unsigned char rtp_payload_type_if_dynamic, FramedSource* input_source) override;

        virtual void closeStreamSource(FramedSource *inputSource) override;
    private:
        framed_source_factory_delegate _framed_source_factory;

        char* fAuxSDPLine;

        char fDoneFlag;

        RTPSink* fDummyRTPSink;

        int _stream_id;
    };
 }}}
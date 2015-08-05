#pragma once
#include "OnDemandServerMediaSubsession.hh"

#include "event_dispatcher_object.hpp"

namespace uvxx { namespace rtsp { namespace details 
{
    class _h264_media_subsession : public OnDemandServerMediaSubsession, public event_dispatcher_object
    {
    public:
        explicit _h264_media_subsession();

        _h264_media_subsession(const _h264_media_subsession&) = delete;

        const _h264_media_subsession& operator=(const _h264_media_subsession&) = delete;

        virtual ~_h264_media_subsession();

        void checkForAuxSDPLine1();

        void afterPlayingDummy1();

    protected:
        void setDoneFlag() { fDoneFlag = ~0; }

    protected:
        virtual char const* getAuxSDPLine(RTPSink* rtpSink, FramedSource* inputSource) override;

        virtual FramedSource* createNewStreamSource(unsigned clientSessionId, unsigned& estBitrate) override;

        virtual RTPSink* createNewRTPSink(Groupsock* rtp_groupsock, unsigned char rtp_payload_type_if_dynamic, FramedSource* input_source) override;

    private:
        char* fAuxSDPLine;

        char fDoneFlag;

        RTPSink* fDummyRTPSink;
    };
 }}}
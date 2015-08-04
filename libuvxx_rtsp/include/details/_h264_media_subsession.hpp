#pragma once
#pragma once
#include <memory>

#include "OnDemandServerMediaSubsession.hh"
#include "event_dispatcher_object.hpp"

namespace uvxx { namespace rtsp { namespace details 
{
    using _usage_environment_ptr = std::shared_ptr<UsageEnvironment>;

    class _h264_media_subsession : public OnDemandServerMediaSubsession, public event_dispatcher_object
    {
    public:
        explicit _h264_media_subsession(const _usage_environment_ptr& usage_environment);

        _h264_media_subsession(const _h264_media_subsession&) = delete;

        const _h264_media_subsession& operator=(const _h264_media_subsession&) = delete;

        virtual ~_h264_media_subsession() = default;

        // Used to implement "getAuxSDPLine()":
        void checkForAuxSDPLine1();

        void afterPlayingDummy1();

    protected:
        void setDoneFlag() { fDoneFlag = ~0; }

    protected: // redefined virtual functions
        virtual char const* getAuxSDPLine(RTPSink* rtpSink, FramedSource* inputSource) override;

        virtual FramedSource* createNewStreamSource(unsigned clientSessionId, unsigned& estBitrate) override;

        virtual RTPSink* createNewRTPSink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource) override;

    private:
        char* fAuxSDPLine;
        char fDoneFlag; // used when setting up "fAuxSDPLine"
        RTPSink* fDummyRTPSink; // ditto
    };
 }}}
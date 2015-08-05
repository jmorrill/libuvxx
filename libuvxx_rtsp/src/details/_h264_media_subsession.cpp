#include "H264VideoRTPSink.hh"
#include "H264VideoStreamDiscreteFramer.hh"

#include "details/_live_common.hpp"
#include "details/_h264_media_subsession.hpp"

using namespace uvxx::rtsp::details;

_h264_media_subsession::_h264_media_subsession() : 
    OnDemandServerMediaSubsession(*_get_live_environment().get(), false)
{
}

_h264_media_subsession::~_h264_media_subsession()
{
}

char const* _h264_media_subsession::getAuxSDPLine(RTPSink* rtp_sink, FramedSource* input_source)
{
    return nullptr;
}

FramedSource* _h264_media_subsession::createNewStreamSource(unsigned client_session_id, unsigned& estimated_kbps)
{
    estimated_kbps = 500; /* kbps, estimate */

    return nullptr; //H264VideoStreamDiscreteFramer::createNew(envir(), nullptr);
}

RTPSink* _h264_media_subsession::createNewRTPSink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* /*inputSource*/) 
{
    return H264VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
}
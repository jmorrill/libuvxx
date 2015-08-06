#include "H264VideoRTPSink.hh"
#include "H264VideoStreamDiscreteFramer.hh"

#include "details/_live_common.hpp"
#include "details/_h264_media_subsession.hpp"

using namespace uvxx::rtsp::details;

_h264_media_subsession::_h264_media_subsession(std::function<FramedSource*(unsigned client_session_id)> factory) :
    OnDemandServerMediaSubsession(*_get_live_environment().get(), false), 
    _framed_source(factory)
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

    auto framesource = H264VideoStreamDiscreteFramer::createNew(envir(), _framed_source(client_session_id));

    return framesource;
}

RTPSink* _h264_media_subsession::createNewRTPSink(Groupsock* rtp_groupsock, unsigned char rtp_payload_type_if_dynamic, FramedSource* /*inputSource*/) 
{
    return H264VideoRTPSink::createNew(envir(), rtp_groupsock, rtp_payload_type_if_dynamic);
}

void _h264_media_subsession::closeStreamSource(FramedSource* inputSource)
{
}
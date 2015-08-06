#include "H264VideoRTPSink.hh"
#include "H264VideoStreamDiscreteFramer.hh"

#include "details/_live_common.hpp"
#include "details/_h264_media_subsession.hpp"

using namespace uvxx::rtsp::details;

_h264_media_subsession::_h264_media_subsession(int stream_id) :
    OnDemandServerMediaSubsession(*_get_live_environment().get(), false), 
    _stream_id(stream_id)
{
   
}

_h264_media_subsession::~_h264_media_subsession()
{
}

void _h264_media_subsession::source_factory_create_set(framed_source_factory_delegate source_factory)
{
    _framed_source_factory = source_factory;
}

char const* _h264_media_subsession::getAuxSDPLine(RTPSink* rtp_sink, FramedSource* input_source)
{
    return nullptr;
}

FramedSource* _h264_media_subsession::createNewStreamSource(unsigned client_session_id, unsigned& estimated_kbps)
{
    if(!_framed_source_factory)
    {
        estimated_kbps = 0;
        return nullptr;
    }

    estimated_kbps = 500; /* kbps, estimate */

    auto source = _framed_source_factory(_stream_id, client_session_id);

    if(source == nullptr)
    {
        return nullptr;
    }
   
    auto framesource = H264VideoStreamDiscreteFramer::createNew(envir(), source);

    return framesource;
}

RTPSink* _h264_media_subsession::createNewRTPSink(Groupsock* rtp_groupsock, unsigned char rtp_payload_type_if_dynamic, FramedSource* /*inputSource*/) 
{
    return H264VideoRTPSink::createNew(envir(), rtp_groupsock, rtp_payload_type_if_dynamic);
}

void _h264_media_subsession::closeStreamSource(FramedSource* inputSource)
{
}
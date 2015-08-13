#include "H264VideoRTPSink.hh"
#include "H264VideoStreamDiscreteFramer.hh"

#include "pplx/pplpp.hpp"
#include "details/_h264_media_subsession.hpp"
#include "details/_h264_framed_source.hpp"

using namespace uvxx::rtsp::details;

void _h264_media_subsession::after_playing_dummy_callback(void* clientData) 
{
    _h264_media_subsession* subsess = static_cast<_h264_media_subsession*>(clientData);

    subsess->after_playing_dummy();
}

_h264_media_subsession::_h264_media_subsession(int stream_id) :
    _live_server_media_subsession(stream_id)
{
   
}

_h264_media_subsession::~_h264_media_subsession()
{
    printf("_h264_media_subsession::~_h264_media_subsession()\n");
}

FramedSource* _h264_media_subsession::createNewStreamSource(unsigned client_session_id, unsigned& estimated_kbps)
{
    _source = std::shared_ptr<_h264_framed_source>(new _h264_framed_source(stream_id(), client_session_id), [](_live_framed_source*)
    {
        /* todo add logic later in case live55 doesn't free */
    });

    estimated_kbps = 4500; /* kbps, estimate */

    auto framesource = H264VideoStreamDiscreteFramer::createNew(envir(), _source.get());

    notify_framed_source(client_session_id);

    return framesource;
}

RTPSink* _h264_media_subsession::createNewRTPSink(Groupsock* rtp_groupsock, unsigned char rtp_payload_type_if_dynamic, FramedSource* /*inputSource*/) 
{
    return H264VideoRTPSink::createNew(envir(), rtp_groupsock, rtp_payload_type_if_dynamic);
}

void _h264_media_subsession::closeStreamSource(FramedSource* inputSource)
{
    OnDemandServerMediaSubsession::closeStreamSource(inputSource);
}

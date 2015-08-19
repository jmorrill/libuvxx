#include "MPEG4ESVideoRTPSink.hh"
#include "MPEG4VideoStreamDiscreteFramer.hh"

#include "pplx/pplpp.hpp"
#include "details/_mpeg4es_media_subsession.hpp"
#include "details/_mpeg4es_framed_source.hpp"
#include "media_attributes.hpp"

using namespace uvxx::rtsp;
using namespace uvxx::rtsp::details;

void _mpeg4es_media_subsession::after_playing_dummy_callback(void* clientData)
{
    _mpeg4es_media_subsession* subsess = static_cast<_mpeg4es_media_subsession*>(clientData);

    subsess->after_playing_dummy();
}

_mpeg4es_media_subsession::_mpeg4es_media_subsession(int stream_id, const uvxx::rtsp::media_attributes& attributes) :
    _live_server_media_subsession(stream_id, attributes)
{

}

_mpeg4es_media_subsession::~_mpeg4es_media_subsession()
{
    printf("_mpeg4es_media_subsession::~_h264_media_subsession()\n");
}

FramedSource* _mpeg4es_media_subsession::createNewStreamSource(unsigned client_session_id, unsigned& estimated_kbps)
{
    _source = std::shared_ptr<_mpeg4es_framed_source>(new _mpeg4es_framed_source(stream_id(), client_session_id), [](_live_framed_source*)
    {
        /* todo add logic later in case live55 doesn't free */
    });

    estimated_kbps = 4500; /* kbps, estimate */

    auto framesource = MPEG4VideoStreamDiscreteFramer::createNew(envir(), _source.get());

    notify_framed_source(client_session_id);

    return framesource;
}

RTPSink* _mpeg4es_media_subsession::createNewRTPSink(Groupsock* rtp_groupsock, unsigned char rtp_payload_type_if_dynamic, FramedSource* /*inputSource*/)
{
    return MPEG4ESVideoRTPSink::createNew(envir(), rtp_groupsock, rtp_payload_type_if_dynamic);
}

void _mpeg4es_media_subsession::closeStreamSource(FramedSource* inputSource)
{
    OnDemandServerMediaSubsession::closeStreamSource(inputSource);
}

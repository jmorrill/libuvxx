#include "H264VideoRTPSink.hh"
#include "H264VideoStreamDiscreteFramer.hh"
#include "pplx/pplpp.hpp"
#include "event_dispatcher_frame.hpp"
#include "details/_live_common.hpp"
#include "details/_h264_media_subsession.hpp"
#include "details/_h264_framed_source.hpp"

using namespace uvxx::rtsp::details;

void _h264_media_subsession::after_playing_dummy_callback(void* clientData) 
{
    _h264_media_subsession* subsess = static_cast<_h264_media_subsession*>(clientData);

    subsess->after_playing_dummy();
}

_h264_media_subsession::_h264_media_subsession(int stream_id) :
    OnDemandServerMediaSubsession(*_get_live_environment().get(), false), 
    _stream_id(stream_id)
{
   
}

_h264_media_subsession::~_h264_media_subsession()
{
    printf("_h264_media_subsession::~_h264_media_subsession()\n");
}

void _h264_media_subsession::framed_source_created_set(framed_source_created_delegate callback)
{
    _framed_source_created = std::move(callback);
}

char const* _h264_media_subsession::getAuxSDPLine(RTPSink* rtp_sink, FramedSource* input_source)
{

    return nullptr;
    check_for_aux_sdp_line();

    if (!_aux_sdp_line.empty())
    {
        return _aux_sdp_line.c_str(); // it's already been set up (for a previous client)
    }

    if (fDummyRTPSink == nullptr) 
    {   // we're not already setting it up for another, concurrent stream
        // Note: For H265 video files, the 'config' information (used for several payload-format
        // specific parameters in the SDP description) isn't known until we start reading the file.
        // This means that "rtpSink"s "auxSDPLine()" will be NULL initially,
        // and we need to start reading data from our file until this changes.
        fDummyRTPSink = rtp_sink;

        // Start reading the file:
        fDummyRTPSink->startPlaying(*input_source, after_playing_dummy_callback, this);

        // Check whether the sink's 'auxSDPLine()' is ready:
    }

    auto dispatcher = event_dispatcher::current_dispatcher();

    printf("begin_invoke\n");

    uvxx::pplx::create_for_loop_task<int>(0, 10, [&](int& iteration)
    {
        printf("create_for_loop_task\n");

        if(check_for_aux_sdp_line())
        {
            iteration = 100;
            printf("create_for_loop_task finished\n");
            return pplx::task_from_result();
        }

        printf("create_for_loop_task continuing\n");

        return uvxx::pplx::create_timer_task(std::chrono::milliseconds(100));
    }).then([&](uvxx::pplx::task<void> t)
    {
        try
        {
            t.get();
        }
        catch (...)
        {
        }
        printf("exiting frame\n");
        _sdp_check_dispatcher_frame.continue_set(false);
    });

    dispatcher.push_frame(_sdp_check_dispatcher_frame);
    printf("finished frame\n");
    return !_aux_sdp_line.empty() ? _aux_sdp_line.c_str() : nullptr;
}

FramedSource* _h264_media_subsession::createNewStreamSource(unsigned client_session_id, unsigned& estimated_kbps)
{
    if(!_source)
    {
        _source = std::shared_ptr<_h264_framed_source>(new _h264_framed_source(_stream_id), [](_live_framed_source*)
        {
            /* todo add logic later in case live55 doesn't free */
        });

        if (_framed_source_created)
        {
            _framed_source_created(_stream_id, client_session_id, _source);
        }
    }

    estimated_kbps = 2500; /* kbps, estimate */

    auto framesource = H264VideoStreamDiscreteFramer::createNew(envir(), _source.get());

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

void _h264_media_subsession::after_playing_dummy()
{
}

bool _h264_media_subsession::check_for_aux_sdp_line()
{
    char const* dasl;

    if (!_aux_sdp_line.empty()) 
    {
        return true;
    }
    else if (fDummyRTPSink != nullptr && (dasl = fDummyRTPSink->auxSDPLine()) != nullptr)
    {
        _aux_sdp_line = dasl;

        fDummyRTPSink = nullptr;

        return true;
    }

    return false;
}
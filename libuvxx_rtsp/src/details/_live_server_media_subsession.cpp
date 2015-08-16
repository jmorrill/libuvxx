#include "H264VideoRTPSink.hh"
#include "pplx/pplpp.hpp"

#include "media_attributes.hpp"
#include "details/_live_common.hpp"
#include "details/_live_server_media_subsession.hpp"

using namespace uvxx::pplx;
using namespace uvxx::rtsp;
using namespace uvxx::rtsp::details;

_live_server_media_subsession::_live_server_media_subsession(int stream_id, const media_attributes& attributes) :
    OnDemandServerMediaSubsession(*_get_live_environment().get(), false),
    _dummy_rtp_sink(nullptr),
    _stream_id(stream_id),
    _attributes(attributes)
{
  
    printf("_live_server_media_subsession ctor %p\n", this);
}

_live_server_media_subsession::~_live_server_media_subsession()
{
    printf("_live_server_media_subsession::~_live_server_media_subsession() %p\n", this);
}

void _live_server_media_subsession::framed_source_created_set(framed_source_created_delegate callback)
{
    _framed_source_created = std::move(callback);
}

task<void> _live_server_media_subsession::preload_sdp_line()
{
    if(!_aux_sdp_line.empty())
    {
        return task_from_result();
    }

    unsigned estimated_kbps = 0;

    auto input_source = std::shared_ptr<FramedSource>(createNewStreamSource(0, estimated_kbps), [=](FramedSource* /*source*/)
    {
        _dummy_rtp_sink = nullptr;
    });

    if (input_source == nullptr)
    {
        return task_from_result(); // not found
    }

    struct in_addr dummy_addr;

    dummy_addr.s_addr = 0;

    std::shared_ptr<Groupsock> dummy_group_sock(createGroupsock(dummy_addr, 0), [](Groupsock* /*group_sock*/)
    {
    });

    unsigned char rtp_payload_type = 96 + static_cast<unsigned char>(trackNumber() - 1); /* if dynamic */

    std::shared_ptr<RTPSink> dummy_rtp_sink(createNewRTPSink(dummy_group_sock.get(), rtp_payload_type, input_source.get()), [=](RTPSink* sink) mutable
    {
        Medium::close(sink);
        
        dummy_group_sock = nullptr;
        
        closeStreamSource(input_source.get());
    });

    if (dummy_rtp_sink != nullptr && dummy_rtp_sink->estimatedBitrate() > 0) estimated_kbps = dummy_rtp_sink->estimatedBitrate();

    if (!_aux_sdp_line.empty())
    {
        return task_from_result(); /* it's already been set up (for a previous client) */
    }

    if (_dummy_rtp_sink == nullptr) 
    {/* we're not already setting it up for another, concurrent stream
        Note: For H265 video files, the 'config' information (used for several payload-format
        specific parameters in the SDP description) isn't known until we start reading the stream.
        This means that "rtpSink"s "auxSDPLine()" will be NULL initially,
        and we need to start reading data from our file until this changes. */
        _dummy_rtp_sink = dummy_rtp_sink.get();

        // Start reading the file:
        _dummy_rtp_sink->startPlaying(*input_source, after_playing_dummy_callback, this);
    }

    return create_for_loop_task<int>(0, 10, [=](int& i)
    {
        check_for_aux_sdp_line();
        
        if(!_aux_sdp_line.empty())
        {
            i = 10; /* break */
            
            return task_from_result();
        }

        return create_timer_task(std::chrono::milliseconds(33));
    }).then([=]
    {
        _dummy_rtp_sink = nullptr;
        
        /* keep alive until input_source */
        auto input = input_source;

        auto group_sock = dummy_group_sock;

        auto sink = dummy_rtp_sink;
        
        return;
    });
}

int _live_server_media_subsession::stream_id()
{
    return _stream_id;
}

char const* _live_server_media_subsession::getAuxSDPLine(RTPSink* /*rtp_sink*/, FramedSource* /*input_source*/)
{
    return _aux_sdp_line.c_str();
}

void _live_server_media_subsession::notify_framed_source(unsigned client_session_id)
{
    if (_framed_source_created)
    {
        _framed_source_created(_stream_id, client_session_id, _source);
    }
}

void _live_server_media_subsession::closeStreamSource(FramedSource* inputSource)
{
    OnDemandServerMediaSubsession::closeStreamSource(inputSource);
}

void _live_server_media_subsession::after_playing_dummy()
{
}

void _live_server_media_subsession::after_playing_dummy_callback(void* /*client_data*/)
{
}

bool _live_server_media_subsession::check_for_aux_sdp_line()
{
    char const* dasl;
    
    if (!_aux_sdp_line.empty())
    {
        return true;
    }
    
    if (_dummy_rtp_sink != nullptr && (dasl = _dummy_rtp_sink->auxSDPLine()) != nullptr)
    {
        _aux_sdp_line = dasl;

        _dummy_rtp_sink = nullptr;

        return true;
    }
    
    return false;
}
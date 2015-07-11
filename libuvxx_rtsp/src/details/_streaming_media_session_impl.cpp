#include "details/_streaming_media_session_impl.hpp"
#include "details/_media_session_impl.hpp"
#include "MediaSession.hh"
#include "details/media_framers/_h264_framer.hpp"

using namespace uvxx::pplx;
using namespace uvxx::rtsp;
using namespace uvxx::rtsp::details;



_streaming_media_session_impl::_streaming_media_session_impl(const media_session& session, 
                                                             std::vector<media_subsession> subsessions) :
    _session(session),
    _subsessions(std::move(subsessions))
{
    int stream_number = 0;

    for (auto& subsession : _subsessions)
    {
        std::shared_ptr<media_framers::_media_framer_base> framer;

        auto codec_name = subsession.codec_name();

        if (codec_name == "H264")
        {
            framer = std::make_shared<media_framers::_h264_framer>(subsession, stream_number);
        }
        else
        {
            framer = std::make_shared<media_framers::_media_framer_base>(subsession, stream_number);
        }

        _media_framers.push_back(framer);

        stream_number++;
    }
}

_streaming_media_session_impl::~_streaming_media_session_impl()
{
    
}

void uvxx::rtsp::details::_streaming_media_session_impl::on_frame_callback_set(std::function<bool(const media_sample&)> callback)
{
    _on_frame_callback = std::move(callback);

    for (auto& framer : _media_framers)
    {
        framer->begin_reading(_on_frame_callback);
    }
}

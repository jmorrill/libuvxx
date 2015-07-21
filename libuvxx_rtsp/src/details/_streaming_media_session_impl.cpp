#include "MediaSession.hh"
#include "media_sample.hpp"
#include "details/_streaming_media_session_impl.hpp"
#include "details/media_framers/_h264_framer.hpp"

using namespace uvxx::pplx;
using namespace uvxx::rtsp;
using namespace uvxx::rtsp::details;
using namespace uvxx::rtsp::details::media_framers;

_streaming_media_session_impl::_streaming_media_session_impl(const media_session& session, 
                                                             std::vector<media_subsession> subsessions) :
    _session(session),
    _subsessions(std::move(subsessions))
{

    for (auto& subsession : _subsessions)
    {
        std::shared_ptr<media_framers::_media_framer_base> framer;

        auto codec_name = subsession.codec_name();

        if (codec_name == "H264")
        {
            framer = std::make_shared<_h264_framer>(subsession);
        }
        else
        {
            framer = std::make_shared<_media_framer_base>(subsession);
        }

        _media_framers.push_back(framer);
    }
}

_streaming_media_session_impl::~_streaming_media_session_impl()
{
    
}

void _streaming_media_session_impl::on_sample_set(read_sample_delegate callback)
{
	_on_sample_callback = std::move(callback);

	for (auto& framer : _media_framers)
	{
		framer->on_sample_set(_on_sample_callback);
	}
}


void _streaming_media_session_impl::read_stream_sample()
{
    for (auto& framer : _media_framers)
    {
        framer->begin_reading();
    }
}

bool _streaming_media_session_impl::operator=(std::nullptr_t rhs)
{
    close();

    return true;
}

void _streaming_media_session_impl::close()
{
    _on_sample_callback = nullptr;

    _media_framers.clear();

    _subsessions.clear();
}

bool _streaming_media_session_impl::operator==(std::nullptr_t rhs)
{
    return !_session ? true : false;
}

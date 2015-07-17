#include "streaming_media_session.hpp"
#include "details/_streaming_media_session_impl.hpp"
#include "media_sample.hpp"

using namespace uvxx::rtsp;
using namespace uvxx::rtsp::details;

streaming_media_session::streaming_media_session()
{

}

streaming_media_session::streaming_media_session(streaming_media_session&& rhs)
{
    *this = std::move(rhs);
}

streaming_media_session::streaming_media_session(const details::_streaming_media_session_impl_ptr& streaming_media_session_impl) :
    __streaming_media_session(streaming_media_session_impl)
{

}

streaming_media_session& streaming_media_session::operator=(streaming_media_session&& rhs)
{
     __streaming_media_session = std::move(rhs.__streaming_media_session);

    return *this;
}

void streaming_media_session::begin_stream_read(std::function<bool(const media_sample&)> callback)
{
    __streaming_media_session->on_frame_callback_set(callback);
}

streaming_media_session::~streaming_media_session()
{

}

bool streaming_media_session::operator=(std::nullptr_t rhs)
{
    if (!__streaming_media_session)
    {
        return true;
    }

    return __streaming_media_session->operator=(rhs);
}

bool streaming_media_session::operator==(std::nullptr_t rhs)
{
    return __streaming_media_session->operator=(rhs);
}

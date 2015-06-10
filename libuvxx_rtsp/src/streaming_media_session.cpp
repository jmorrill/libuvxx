#include "streaming_media_session.hpp"

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

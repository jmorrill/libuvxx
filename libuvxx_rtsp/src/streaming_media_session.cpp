#include "streaming_media_session.hpp"
#include "details/_streaming_media_session_impl.hpp"

using namespace uvxx::rtsp;
using namespace uvxx::rtsp::details;

streaming_media_session::streaming_media_session()
{

}

streaming_media_session::streaming_media_session(streaming_media_session&& rhs)
{
    *this = std::move(rhs);
}

streaming_media_session::streaming_media_session(const _streaming_media_session_impl_ptr& streaming_media_session_impl) :
    __streaming_media_session(streaming_media_session_impl)
{

}

streaming_media_session& streaming_media_session::operator=(streaming_media_session&& rhs)
{
    if(this != &rhs)
    {
        __streaming_media_session = std::move(rhs.__streaming_media_session);
    }

    return *this;
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

streaming_media_session::operator bool() const
{
    return __streaming_media_session != nullptr;
}

void streaming_media_session::read_stream_sample()
{
    __streaming_media_session->read_stream_sample();
}

void streaming_media_session::on_sample_callback_set(read_sample_delegate callback)
{
    __streaming_media_session->on_sample_set(std::move(callback));
}

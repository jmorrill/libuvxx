#include "server_media_session.hpp"
#include "details/_server_media_session_impl.hpp"

using namespace uvxx::rtsp;

server_media_session::server_media_session()
{
    __server_media_session_impl = std::make_shared<details::_server_media_session_impl>();
}

server_media_session::server_media_session(const server_media_session& session)
{
    __server_media_session_impl = session.__server_media_session_impl;
}

server_media_session& server_media_session::operator=(const server_media_session& session)
{
    __server_media_session_impl = session.__server_media_session_impl;

    return *this;
}

server_media_session::server_media_session(server_media_session&& session)
{
    *this = std::move(session);
}

server_media_session& server_media_session::operator=(server_media_session&& session)
{
    __server_media_session_impl = std::move(session.__server_media_session_impl);

    return *this;
}

void server_media_session::set_media_descriptor(const media_descriptor& descriptor)
{
    __server_media_session_impl->set_media_descriptor(descriptor);
}
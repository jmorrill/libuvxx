#include "server_media_session.hpp"
#include "details/_server_media_session_impl.hpp"

using namespace uvxx::rtsp;

server_media_session::server_media_session()
{
    __server_media_session_impl = std::make_shared<details::_server_media_session_impl>();
}
#include "details/_live_server_media_session.hpp"

using namespace uvxx::rtsp::details;

_live_server_media_session::_live_server_media_session(const _usage_environment_ptr& environment) : 
    ServerMediaSession(*(environment.get()), "stream", nullptr, nullptr, false, nullptr),
    _usage_environment(environment)
{
   
}

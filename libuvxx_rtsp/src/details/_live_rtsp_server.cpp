#include "details/_live_rtsp_server.hpp"

using namespace uvxx::rtsp::details;

_live_rtsp_server::_live_rtsp_server(const _usage_environment_ptr& environment, uint16_t port) : 
    RTSPServerSupportingHTTPStreaming(*environment.get(), setup_socket(environment, port), 
        port, 
        nullptr, 
        65)
{
}

_live_rtsp_server::~_live_rtsp_server()
{
}

void _live_rtsp_server::set_on_lookup_media_session(_lookup_media_session_delegate callback)
{
    __lookup_media_session_delegate = callback;
}

ServerMediaSession* _live_rtsp_server::lookupServerMediaSession(char const* stream_name, Boolean /*is_first_lookup_in_session*/)
{
    return __lookup_media_session_delegate ? __lookup_media_session_delegate(stream_name) : nullptr;
}

int _live_rtsp_server::setup_socket(const _usage_environment_ptr& environment, uint16_t port)
{
    Port port_(port);

    auto socket = setUpOurSocket(*environment.get(), port_);

    return socket;
}

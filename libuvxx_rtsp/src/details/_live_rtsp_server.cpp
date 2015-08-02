#include "details/_live_rtsp_server.hpp"

using namespace uvxx::rtsp::details;

_live_rtsp_server::_live_rtsp_server(const _usage_environment_ptr& environment, int port) : 
    RTSPServerSupportingHTTPStreaming(*environment.get(), setup_socket(environment, port), 
        port, 
        nullptr, 
        65)
{
}


_live_rtsp_server::~_live_rtsp_server()
{
}

ServerMediaSession* _live_rtsp_server::lookupServerMediaSession(char const* streamName, Boolean isFirstLookupInSession)
{
    return RTSPServer::lookupServerMediaSession(streamName, isFirstLookupInSession);
}

int _live_rtsp_server::setup_socket(const _usage_environment_ptr& environment, int port)
{
    Port port_ = port;

    auto socket = setUpOurSocket(*environment.get(), port_);

    return socket;
}

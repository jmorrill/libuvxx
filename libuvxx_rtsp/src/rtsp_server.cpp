#include "rtsp_server.hpp"
#include "details/_rtsp_server_impl.hpp"

using namespace uvxx::rtsp;
using namespace uvxx::rtsp::details;

rtsp_server::rtsp_server()
{
}

int rtsp_server::port() const
{
	return __rtsp_server_impl ? __rtsp_server_impl->port() : 0;
}

void rtsp_server::start_server(int port)
{
    __rtsp_server_impl = std::make_shared<_rtsp_server_impl>(port);
}
#include "rtsp_server.hpp"
#include "details/_rtsp_server_impl.hpp"

using namespace uvxx::rtsp;
using namespace uvxx::rtsp::details;

rtsp_server::rtsp_server()
{
}

uint16_t rtsp_server::port() const
{
	return private_impl() ? private_impl()->port() : 0;
}

void rtsp_server::start_server(uint16_t port)
{
    private_impl_set(std::make_shared<_rtsp_server_impl>(port));
}

void rtsp_server::on_session_request_set(on_session_request_delegate callback)
{
    private_impl()->on_session_request_set(callback);
}
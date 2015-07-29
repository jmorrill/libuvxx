#include "rtsp_server.hpp"
#include "details/_rtsp_server_impl.hpp"

using namespace uvxx::rtsp;
using namespace uvxx::rtsp::details;

rtsp_server::rtsp_server()
{
}

rtsp_server::rtsp_server(const rtsp_server& client)
{
}
//
//rtsp_server::rtsp_server(rtsp_server&& rhs)
//{
//}
//
//rtsp_server& rtsp_server::operator=(rtsp_server&& rhs)
//{
//    
//}

int rtsp_server::port_get() const
{
    return -1;
}

void rtsp_server::start_server(int port)
{
    __rtsp_server_impl = std::make_shared<_rtsp_server_impl>(port);
}
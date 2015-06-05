#include "details/_rtsp_client_impl.hpp"
#include "rtsp_client.hpp"

using namespace uvxx::rtsp;
using namespace uvxx::rtsp::details;

uvxx::rtsp::rtsp_client::rtsp_client()
{
    __rtsp_client_imp = std::shared_ptr<_rtsp_client_impl>(new _rtsp_client_impl);
}

rtsp_client& uvxx::rtsp::rtsp_client::operator=(rtsp_client&& rhs)
{
    __rtsp_client_imp = std::move(rhs.__rtsp_client_imp);
    return *this;
}

uvxx::rtsp::rtsp_client::rtsp_client(rtsp_client&& rhs)
{
     *this = std::move(rhs);
}

uvxx::pplx::task<void> uvxx::rtsp::rtsp_client::open(const std::string& url)
{
    return __rtsp_client_imp->open(url);
}


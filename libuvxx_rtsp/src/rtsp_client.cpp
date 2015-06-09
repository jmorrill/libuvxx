#include "details/_rtsp_client_impl.hpp"
#include "rtsp_client.hpp"

using namespace uvxx::pplx;
using namespace uvxx::rtsp;
using namespace uvxx::rtsp::details;

rtsp_client::rtsp_client()
{
    __rtsp_client_imp = _rtsp_client_impl_ptr(new _rtsp_client_impl);
}

rtsp_client& rtsp_client::operator=(rtsp_client&& rhs)
{
    __rtsp_client_imp = std::move(rhs.__rtsp_client_imp);
    return *this;
}

rtsp_client::rtsp_client(rtsp_client&& rhs)
{
     *this = std::move(rhs);
}

task<void> rtsp_client::open(const std::string& url) const
{
    return __rtsp_client_imp->open(url);
}

media_session rtsp_client::media_session_get() const
{
    return media_session(__rtsp_client_imp->media_session_get());
}

task<void> rtsp_client::play() const
{
    return __rtsp_client_imp->play(media_session_get().subsessions_get());
}


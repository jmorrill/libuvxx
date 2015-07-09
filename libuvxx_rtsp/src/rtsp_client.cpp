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
    auto this_ptr = *this;

    return __rtsp_client_imp->open(url).then([this_ptr]
    {
         auto tid = std::this_thread::get_id();
    });;
}

media_session rtsp_client::media_session() const
{
    return __rtsp_client_imp->session();
}

task<void> rtsp_client::play() const
{
    auto this_ptr = *this;

    return __rtsp_client_imp->play(media_session().subsessions())
    .then([this_ptr]
    {
        return;
    });
}

task<void> rtsp_client::play(std::vector<media_subsession> media_sessions) const
{
    auto this_ptr = *this;

    return __rtsp_client_imp->play(std::move(media_sessions))
    .then([this_ptr]
    {
        return;
    });
}

std::string uvxx::rtsp::rtsp_client::username() const
{
    return __rtsp_client_imp->username();
}

void uvxx::rtsp::rtsp_client::username_set(const std::string& username)
{
    __rtsp_client_imp->username_set(username);
}

std::string uvxx::rtsp::rtsp_client::password() const
{
    return __rtsp_client_imp->password();
}

void uvxx::rtsp::rtsp_client::password_set(const std::string& password)
{
    __rtsp_client_imp->password_set(password);
}

uvxx::rtsp::transport_protocol uvxx::rtsp::rtsp_client::protocol() const
{
    return __rtsp_client_imp->protocol();
}

void uvxx::rtsp::rtsp_client::protocol_set(uvxx::rtsp::transport_protocol protocol)
{
    __rtsp_client_imp->protocol_set(protocol);
}

void rtsp_client::begin_stream_read(read_stream_delegate call_back) const
{
    __rtsp_client_imp->begin_stream_read(call_back);
}


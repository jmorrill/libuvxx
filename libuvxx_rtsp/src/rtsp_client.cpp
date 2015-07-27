#include "rtsp_client.hpp"
#include "media_session.hpp"
#include "details/_rtsp_client_impl.hpp"

using namespace uvxx::pplx;
using namespace uvxx::rtsp;
using namespace uvxx::rtsp::details;

rtsp_client::rtsp_client()
{
    __rtsp_client_imp = std::make_shared<_rtsp_client_impl>();
}

rtsp_client& rtsp_client::operator=(rtsp_client&& rhs)
{
    if(this != &rhs)
    {
        __rtsp_client_imp = std::move(rhs.__rtsp_client_imp);
    }

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

media_session rtsp_client::session() const
{
    return __rtsp_client_imp->session();
}

task<void> rtsp_client::play() const
{
    return __rtsp_client_imp->play(session().subsessions());
}

task<void> rtsp_client::play(std::vector<media_subsession> media_sessions) const
{
    return __rtsp_client_imp->play(std::move(media_sessions));
}

std::string rtsp_client::username() const
{
    return __rtsp_client_imp->username();
}

void rtsp_client::credentials_set(const std::string& user_name, const std::string& pass)
{
    __rtsp_client_imp->credentials_set(user_name, pass);
}

std::string rtsp_client::password() const
{
    return __rtsp_client_imp->password();
}

uvxx::rtsp::transport_protocol rtsp_client::protocol() const
{
    return __rtsp_client_imp->protocol();
}

void rtsp_client::protocol_set(transport_protocol protocol)
{
    __rtsp_client_imp->protocol_set(protocol);
}

void rtsp_client::timeout_set(std::chrono::milliseconds timeout)
{
    __rtsp_client_imp->timeout_set(timeout);
}

std::chrono::milliseconds rtsp_client::timeout() const
{
    return __rtsp_client_imp->timeout();
}

stream_statistics rtsp_client::stream_statistics_get(int stream_id) const
{
    return __rtsp_client_imp->stream_statistics_get(stream_id);
}

void rtsp_client::read_stream_sample() const
{
    __rtsp_client_imp->read_stream_sample();
}

void rtsp_client::on_sample_set(read_sample_delegate callback) const
{
    __rtsp_client_imp->on_sample_callback_set(std::move(callback));
}

void rtsp_client::on_stream_closed_set(stream_closed_delegate callback) const
{
	__rtsp_client_imp->on_stream_closed_set(callback);
}


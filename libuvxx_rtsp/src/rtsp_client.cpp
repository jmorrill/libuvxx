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

void rtsp_client::read_stream_sample() const
{
    __rtsp_client_imp->read_stream_sample();
}

void rtsp_client::on_sample_set(read_sample_delegate callback) const
{
	__rtsp_client_imp->on_sample_callback_set(std::move(callback));
}



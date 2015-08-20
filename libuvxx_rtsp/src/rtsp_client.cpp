#include "rtsp_client.hpp"
#include "media_session.hpp"
#include "details/_rtsp_client_impl.hpp"

using namespace uvxx::pplx;
using namespace uvxx::rtsp;
using namespace uvxx::rtsp::details;

rtsp_client::rtsp_client()
{
    private_impl_set(std::make_shared<_rtsp_client_impl>());
}

task<void> rtsp_client::open(const std::string& url) const
{
    return private_impl()->open(url);
}

media_session rtsp_client::session() const
{
    return private_impl()->session();
}

task<void> rtsp_client::play() const
{
    return private_impl()->play(session().subsessions());
}

task<void> rtsp_client::play(std::vector<media_subsession> media_sessions) const
{
    return private_impl()->play(std::move(media_sessions));
}

std::string rtsp_client::username() const
{
    return private_impl()->username();
}

void rtsp_client::credentials_set(const std::string& user_name, const std::string& pass)
{
    private_impl()->credentials_set(user_name, pass);
}

std::string rtsp_client::password() const
{
    return private_impl()->password();
}

uvxx::rtsp::transport_protocol rtsp_client::protocol() const
{
    return private_impl()->protocol();
}

void rtsp_client::protocol_set(transport_protocol protocol)
{
    private_impl()->protocol_set(protocol);
}

void rtsp_client::timeout_set(std::chrono::milliseconds timeout)
{
    private_impl()->timeout_set(timeout);
}

std::chrono::milliseconds rtsp_client::timeout() const
{
    return private_impl()->timeout();
}

stream_statistics rtsp_client::stream_statistics_get(int stream_id) const
{
    return private_impl()->stream_statistics_get(stream_id);
}

media_descriptor rtsp_client::media_descriptor_get()
{
    return private_impl()->media_descriptor_get();
}

void rtsp_client::read_stream_sample() const
{
    private_impl()->read_stream_sample();
}

void rtsp_client::on_sample_set(read_sample_delegate callback) const
{
    private_impl()->on_sample_callback_set(std::move(callback));
}

void rtsp_client::on_stream_closed_set(stream_closed_delegate callback) const
{
    private_impl()->on_stream_closed_set(callback);
}


#include "media_session.hpp"
#include "details/_media_session_impl.hpp"

using namespace uvxx::rtsp;
using namespace uvxx::rtsp::details;

media_subsession::media_subsession(const _media_subsession_impl_ptr& _media_subsession)
{
    private_impl_set(_media_subsession);
}

media_subsession::media_subsession()
{

}

details::_media_subsession_impl_ptr& media_subsession::media_subsession_impl_get()
{
    return private_impl();
}

std::string media_subsession::codec_name() const
{
    return private_impl()->codec_name();
}

std::string media_subsession::get_attribute(const std::string& attribute_name) const
{
    return private_impl()->get_attribute(attribute_name);
}

int media_subsession::stream_number() const
{
    return private_impl()->stream_number();
}

std::string media_subsession::medium_name() const
{
    return private_impl()->medium_name();
}

uint32_t media_subsession::rtp_timestamp_frequency()
{
    return private_impl()->rtp_timestamp_frequency();
}

uint32_t media_subsession::channel_count() const
{
    return private_impl()->channel_count();
}

media_session::media_session()
{

}

media_session::media_session(const _media_session_impl_ptr& _media_session) 
{
    private_impl_set(_media_session);
}

size_t media_session::subsession_count() const
{
    return private_impl()->subsessions().size();
}

 media_subsession media_session::subsession(size_t index) const
{
    return media_subsession(private_impl()->subsessions().at(index));
}

std::vector<media_subsession> media_session::subsessions() const
{
    auto subsessioncount = subsession_count();

    std::vector<media_subsession> subsessions;

    for (size_t i = 0; i < subsessioncount; i++)
    {
        subsessions.emplace_back(subsession(i));
    }

    return subsessions;
}

media_session::~media_session()
{

}

#include "media_session.hpp"
#include "details/_media_session_impl.hpp"

using namespace uvxx::rtsp;
using namespace uvxx::rtsp::details;

media_subsession::media_subsession(const _media_subsession_impl_ptr& _media_subsession)
{
    __media_subsession = _media_subsession;
}

media_subsession::media_subsession(media_subsession&& rhs)
{
    *this = std::move(rhs);
}

media_subsession::media_subsession()
{

}

bool media_subsession::operator=(std::nullptr_t rhs)
{
    __media_subsession.reset();
    return true;
}

bool media_subsession::operator==(std::nullptr_t rhs)
{
    return __media_subsession.get() != nullptr;
}


media_subsession& media_subsession::operator=(media_subsession&& rhs)
{
    __media_subsession = std::move(rhs.__media_subsession);
    return *this;
}

const std::string media_subsession::codec_name() const
{
    return __media_subsession->codec_name();
}

const std::string uvxx::rtsp::media_subsession::get_attribute(const std::string& attribute_name) const
{
    return __media_subsession->get_attribute(attribute_name);
}

int uvxx::rtsp::media_subsession::stream_number() const
{
    return __media_subsession->stream_number();
}

media_session::media_session(media_session&& rhs)
{
    *this = std::move(rhs);
}

media_session::media_session()
{

}

media_session::media_session(const _media_session_impl_ptr& _media_session) : __media_session(_media_session)
{

}

media_session& media_session::operator=(media_session&& rhs)
{
    __media_session = std::move(rhs.__media_session);
    return *this;
}

const size_t media_session::subsession_count() const
{
    return __media_session->subsessions().size();
}

const media_subsession media_session::subsession(size_t index) const
{
    return media_subsession(__media_session->subsessions().at(index));
}

const std::vector<media_subsession> media_session::subsessions() const
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

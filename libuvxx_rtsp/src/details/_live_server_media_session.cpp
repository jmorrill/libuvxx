#include <functional>
#include "details/_live_common.hpp"
#include "details/_live_server_media_session.hpp"

using namespace uvxx::rtsp::details;

_live_server_media_session::_live_server_media_session() : 
    ServerMediaSession(*(_get_live_environment().get()), "stream", nullptr, nullptr, false, nullptr),
    _is_externally_owned(false)
{
    deleteWhenUnreferenced() = true;
}

_live_server_media_session::~_live_server_media_session()
{
    if(__live_session_closed_delegate)
    {
        __live_session_closed_delegate();
    }
}

bool _live_server_media_session::is_externally_owned() const
{
    return _is_externally_owned;
}

void _live_server_media_session::is_externally_owned_set(bool is_externally_owned)
{
    _is_externally_owned = is_externally_owned;
}

void _live_server_media_session::on_session_closed(_live_session_closed_delegate callback)
{
    __live_session_closed_delegate = callback;
}
#include "BasicUsageEnvironment.hh"
#include "details/_live_common.hpp"
#include "details/_live_server_media_session.hpp"
#include "details/_server_media_session_impl.hpp"
#include "details/_uvxx_task_scheduler.hpp"

using namespace uvxx::rtsp::details;

_server_media_session_impl::_server_media_session_impl()
{
    _usage_environment = _get_live_environment();

    __live_server_media_session = _live_server_media_session_ptr(new _live_server_media_session(_usage_environment),
    [](_live_server_media_session* session)
    {
        if(!session->is_externally_owned())
        {
            delete session;
        }
    });

    __live_server_media_session->on_session_closed(std::bind(&_server_media_session_impl::on_session_closed, this));
}

_server_media_session_impl::~_server_media_session_impl()
{
    __live_server_media_session->on_session_closed(nullptr);
}

void _server_media_session_impl::on_session_closed()
{
    __live_server_media_session.reset(static_cast<_live_server_media_session*>(nullptr));
}
#pragma once
#include "media_descriptor.hpp"
#include "_live_server_media_subsession.hpp"

namespace uvxx { namespace rtsp { namespace details
{
    _live_server_media_subsession*  _create_server_media_subsession(const stream_descriptor& descriptor);
}}}
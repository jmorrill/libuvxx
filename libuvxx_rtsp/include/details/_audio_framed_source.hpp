#pragma once
#include "_live_framed_source.hpp"
#include "SimpleRTPSource.hh"
#include "_live_common.hpp"

namespace uvxx { namespace rtsp { namespace details
{
    class _audio_framed_source : public _live_framed_source
    {
    public:
        _audio_framed_source(int stream_id, unsigned client_session_id)
            : _live_framed_source(stream_id, client_session_id)
        {
        }
    };
}}}
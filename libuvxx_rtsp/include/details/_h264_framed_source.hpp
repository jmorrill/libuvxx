#pragma once

#include "_live_framed_source.hpp"

namespace uvxx { namespace rtsp { namespace details
{
    class _h264_framed_source : public _live_framed_source
    {
    public:
        _h264_framed_source(int stream_id) : _live_framed_source(stream_id)
        {
        }
    };
}}}
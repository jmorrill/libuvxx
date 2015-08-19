#pragma once
#include <chrono>
#include "_live_framed_source.hpp"

namespace uvxx { namespace rtsp { namespace details
{
    class _mpeg4es_framed_source : public _live_framed_source
    {
    public:
        _mpeg4es_framed_source(int stream_id, unsigned session_id);

        virtual ~_mpeg4es_framed_source();
    };
}}}
#include "details/_live_framed_source.hpp"
#include "details/_live_common.hpp"

using namespace uvxx::rtsp::details;

_live_framed_source::_live_framed_source(int stream_id) : 
    FramedSource(*_get_live_environment().get()),
    _stream_id(stream_id)
{
    _payload = new unsigned char[400000];
    fTo = nullptr;
}

_live_framed_source::~_live_framed_source()
{
    if (_on_source_closed)
    {
        _on_source_closed(_stream_id);
    }
}

void _live_framed_source::on_closed_set(_framed_source_closed_delegate source_closed)
{
    _on_source_closed = std::move(source_closed);
}
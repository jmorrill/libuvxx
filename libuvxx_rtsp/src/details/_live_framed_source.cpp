#include "details/_live_framed_source.hpp"
#include "details/_live_common.hpp"

using namespace uvxx::rtsp::details;

_live_framed_source::_live_framed_source(int stream_id) : 
    FramedSource(*_get_live_environment().get()),
    _stream_id(stream_id)
{
    fTo = nullptr;
}

_live_framed_source::~_live_framed_source()
{
    if (_on_source_closed)
    {
        _on_source_closed(_stream_id);
    }
}

int _live_framed_source::stream_id()
{
    return _stream_id;
}

void _live_framed_source::on_closed_set(_framed_source_closed_delegate source_closed)
{
    _on_source_closed = std::move(source_closed);
}

void _live_framed_source::deliver_sample(const uvxx::rtsp::media_sample& sample)
{
    if (!isCurrentlyAwaitingData())
    {
        return;
    }


    deliver_sample_override(sample);
}

#undef min

void _live_framed_source::deliver_sample_override(const uvxx::rtsp::media_sample& sample)
{
    size_t bytes_to_copy = std::min(fMaxSize, sample.size());

    fNumTruncatedBytes = sample.size() - bytes_to_copy;

    memcpy(fTo, const_cast<unsigned char*>(sample.data()), bytes_to_copy);

    fFrameSize = sample.size();

    FramedSource::afterGetting(this);
}

void _live_framed_source::doGetNextFrame()
{
           
}
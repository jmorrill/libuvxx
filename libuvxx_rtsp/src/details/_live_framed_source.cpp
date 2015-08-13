#include "details/_live_framed_source.hpp"
#include "details/_live_common.hpp"
#include <GroupsockHelper.hh>

using namespace uvxx::rtsp::details;

_live_framed_source::_live_framed_source(int stream_id, unsigned client_session_id) : 
    FramedSource(*_get_live_environment().get()),
    _busy_delivering(false),
    _stream_id(stream_id),
    _client_session_id(client_session_id),
    _is_first_sample(true)
{
    fTo = nullptr;
}

_live_framed_source::~_live_framed_source()
{
    if (_on_source_closed)
    {
        _on_source_closed(_stream_id, _client_session_id);
    }
}

int _live_framed_source::stream_id()
{
    return _stream_id;
}

unsigned _live_framed_source::session_id()
{
    return _client_session_id;
}

void _live_framed_source::on_closed_set(_framed_source_closed_delegate source_closed)
{
    _on_source_closed = std::move(source_closed);
}

void _live_framed_source::deliver_sample(const uvxx::rtsp::media_sample& sample)
{
    if (!isCurrentlyAwaitingData() || _busy_delivering)
    {
        return;
    }

    auto current_presentation_time = sample.presentation_time();

    if (_is_first_sample)
    {
        struct timeval time_now_val;

        gettimeofday(&time_now_val, nullptr);

        std::chrono::microseconds time_now(1000000ll * time_now_val.tv_sec + time_now_val.tv_usec);

        _presentation_time_base = time_now - current_presentation_time;

        _is_first_sample = false;
    }

    auto presentation_time = _presentation_time_base + current_presentation_time;

    fPresentationTime.tv_sec = static_cast<long>(presentation_time.count() / 1000000);

    fPresentationTime.tv_usec = static_cast<long>(presentation_time.count() % 1000000);

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
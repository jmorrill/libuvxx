#include "details/_live_media_sink.hpp"

#define SINK_RECEIVE_BUFFER_SIZE 1000000

using namespace uvxx::rtsp::details;

_live_media_sink::_live_media_sink(UsageEnvironment& env) : MediaSink(env)
{
    _recv_buffer.resize(SINK_RECEIVE_BUFFER_SIZE);
}

_live_media_sink::~_live_media_sink()
{

}

void _live_media_sink::afterGettingFrame(void* client_data, unsigned frame_size, unsigned truncated_bytes, struct timeval presentation_time, unsigned duration_microseconds)
{

}

void _live_media_sink::afterGettingFrame(unsigned frame_size, unsigned truncated_bytes, struct timeval presentation_time, unsigned duration_microseconds)
{
    _live_media_sink_frame frame = {frame_size, truncated_bytes, presentation_time, duration_microseconds};

    _frame_callback(frame);

    continuePlaying();
}

Boolean _live_media_sink::continuePlaying()
{
    if (!fSource) 
    {
        return false; /* sanity check (should not happen) */
    }

    /* Request the next frame of data from our input source.  "afterGettingFrame()" will get called later, when it arrives */
    fSource->getNextFrame(_recv_buffer.data(), 
                          SINK_RECEIVE_BUFFER_SIZE,
                          afterGettingFrame, 
                          this,
                          onSourceClosure, 
                          this);
    return True;
}

void uvxx::rtsp::details::_live_media_sink::on_frame_callback_set(_live_media_sink_frame_callback callback)
{
    _frame_callback = callback;
}

void uvxx::rtsp::details::_live_media_sink::buffer_size_set(size_t size)
{
    _recv_buffer.resize(size);
}

#pragma once
#include "MediaSink.hh"
#include "MediaSession.hh"
#include <memory>
#include <vector>
#include <functional>

namespace uvxx { namespace rtsp { namespace details 
{
    struct _live_media_sink_frame
    {
        unsigned frame_size;
        unsigned truncated_bytes;
        struct timeval presentation_time;
        unsigned duration_microseconds;
    };

    using _live_media_sink_frame_callback = std::function<void(const _live_media_sink_frame&)>;

    class _live_media_sink : public MediaSink
    {
    public:
        _live_media_sink(UsageEnvironment& env);

    public:

        void on_frame_callback_set(_live_media_sink_frame_callback callback);

        void buffer_size_set(size_t size);
            
    private:
        virtual ~_live_media_sink();

        static void afterGettingFrame(void* client_data, unsigned frame_size, unsigned truncated_bytes, struct timeval presentation_time, unsigned duration_microseconds);

        void afterGettingFrame(unsigned frame_size, unsigned truncated_bytes, struct timeval presentation_time, unsigned duration_microseconds);
        
        virtual Boolean continuePlaying();

    private:
        std::vector<u_int8_t> _recv_buffer;

        _live_media_sink_frame_callback _frame_callback;
    };
}}}
#pragma once
#include "media_session.hpp"
#include "details/_media_session_impl.hpp"
#include "media_sample.hpp"

namespace uvxx { namespace rtsp { namespace details { namespace media_framers
{
    class _media_framer_base
    {
    public:
        _media_framer_base(const media_subsession& subsession, int stream_number);

        virtual ~_media_framer_base();

    public:
        void begin_reading(std::function<bool(const media_sample&)> callback);
    
        int stream_number();

    protected:

        virtual void sample_receieved();

        uvxx::rtsp::media_sample sample();

        void do_sample_callback();

        media_subsession _subsession;

        void continue_reading();

    private:
        static void on_after_getting_frame(void* client_data, unsigned packet_data_size, unsigned truncated_bytes, struct timeval presentation_time, unsigned duration_in_microseconds);

        static void on_rtcp_bye(void* client_data);

        void on_after_getting_frame(unsigned packet_data_size, unsigned truncated_bytes, struct timeval presentation_time, unsigned duration_in_microseconds);

        static void adjust_buffer_for_trucated_bytes(unsigned truncated_amount, const uvxx::rtsp::media_sample& sample);

    private:
        uvxx::rtsp::media_sample _sample;

        std::function<bool(const media_sample&)> _sample_callback;

        std::chrono::microseconds _lastPresentationTime;

        std::chrono::microseconds _currentPresentationTime;

        int _stream_number;

        bool _was_synced;
    };
}}}}
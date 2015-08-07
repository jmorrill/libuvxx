#pragma once

#include "_live_framed_source.hpp"

namespace uvxx { namespace rtsp { namespace details
{
    class _h264_framed_source : public _live_framed_source
    {
        enum class nal_to_deliver
        {
            none = 0,
            sps,
            pps,
            payload
        };

    public:
        _h264_framed_source(int stream_id);

        virtual ~_h264_framed_source() = default;

    protected:
        virtual void deliver_sample_override(const media_sample& sample) override;

        virtual void doGetNextFrame() override;

    private:
        void do_iterative_nal_delivery();

    private:
        bool _busy_delivering;

        nal_to_deliver _nal_to_deliver;

        std::vector<uint8_t> _payload;

        size_t _payload_size;

        uvxx::io::memory_buffer _sps;

        uvxx::io::memory_buffer _pps;

        std::chrono::microseconds _presentation_time;
    };
}}}
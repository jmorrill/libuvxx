#pragma once
#include <chrono>

class RTPReceptionStats;

namespace uvxx { namespace rtsp { namespace details { namespace media_framers 
{
    class _qos_stats
    {
    public:
        explicit _qos_stats();

        _qos_stats(const _qos_stats&) = default;

        _qos_stats& operator=(const _qos_stats&) = default;

    public:
        void reset_statistics_interval(std::chrono::milliseconds reset_interval);

        std::chrono::milliseconds reset_statistics_interval();

        double percent_packet_loss() const;

        uint32_t expected_packet_count() const;

        uint32_t current_received_packet_count() const;

        double total_kbytes_received() const;

        void record_stats(RTPReceptionStats& stats);

    private:
        double _total_kbytes_received;

        double _percent_packet_loss;

        uint32_t _current_expected_packet_count;

        uint32_t _current_received_packet_count;

        int64_t _last_expected_packet_count;

        int64_t _last_received_packet_count;

        std::chrono::milliseconds _reset_statistics_interval;

        std::chrono::high_resolution_clock::time_point _last_check_time;
    };
}}}}
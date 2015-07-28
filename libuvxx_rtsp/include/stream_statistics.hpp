#pragma once
#include <chrono>

namespace uvxx { namespace rtsp
{
    struct stream_statistics
    {
        double total_kbytes_received;

        double percent_packet_loss;

        uint32_t expected_packet_count;

        uint32_t received_packet_count;

        std::chrono::milliseconds statistics_window;
    };
}}
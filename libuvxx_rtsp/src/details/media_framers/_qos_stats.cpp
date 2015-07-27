#include <complex> 
#include "RTPSource.hh"

#include "details/media_framers/_qos_stats.hpp"

using namespace uvxx::rtsp::details::media_framers;

_qos_stats::_qos_stats(): _percent_packet_loss(0),
                        _current_expected_packet_count(0),
                        _current_received_packet_count(0),
                        _last_expected_packet_count(0),
                        _last_received_packet_count(0),
                        _reset_statistics_interval(10)
{
}

void _qos_stats::reset_statistics_interval(std::chrono::milliseconds reset_interval)
{
    _reset_statistics_interval = reset_interval;
}

std::chrono::milliseconds _qos_stats::reset_statistics_interval()
{
    return _reset_statistics_interval;
}

double _qos_stats::percent_packet_loss() const
{
    return _percent_packet_loss;
}

uint32_t _qos_stats::expected_packet_count() const
{
    return _current_expected_packet_count;
}

uint32_t _qos_stats::current_received_packet_count() const
{
    return _current_received_packet_count;
}

double _qos_stats::total_kbytes_received() const
{
    return _total_kbytes_received;
}

void _qos_stats::record_stats(RTPReceptionStats& stats)
{
    auto now = std::chrono::high_resolution_clock::now();

    bool time_to_reset = true;

    if (now - _last_check_time < std::chrono::seconds(10))
    {
        time_to_reset = false;
    }

    _total_kbytes_received = stats.totNumKBytesReceived();

    _current_received_packet_count = stats.totNumPacketsReceived();

    _current_expected_packet_count = stats.totNumPacketsExpected();

    auto expected_packet_count = _current_expected_packet_count - _last_expected_packet_count;

    auto received_packet_count = _current_received_packet_count - _last_received_packet_count;

    _percent_packet_loss = 100.0 - ((received_packet_count / static_cast<double>(expected_packet_count)) * 100.0);

    if (_percent_packet_loss < 0)
    {
        _percent_packet_loss = std::abs(_percent_packet_loss);
    }

    if (time_to_reset)
    {
        _last_expected_packet_count = _current_expected_packet_count;

        _last_received_packet_count = _current_received_packet_count;

        _last_check_time = std::chrono::high_resolution_clock::now();
    }

    if (_percent_packet_loss > 0)
    {
        printf("packet loss %f\n", _percent_packet_loss);
    }
}
#include "details/_media_sample_impl.hpp"

using namespace uvxx::rtsp;
using namespace uvxx::rtsp::details;

_media_sample_impl::_media_sample_impl() :
                        _size(0),
                        _stream_number(0),
                        _presentation_time(0),
                        _is_truncated(false)
{
    
}

const size_t _media_sample_impl::size() const
{
    return _size;
}

void _media_sample_impl::size_set(size_t size)
{
    _size = size;
}

void _media_sample_impl::presentation_time_set(std::chrono::microseconds presentation_time)
{
    _presentation_time = presentation_time;
}

void _media_sample_impl::is_complete_sample_set(bool complete_sample)
{
    _is_complete_sample = complete_sample;
}

void _media_sample_impl::is_truncated_set(bool truncated)
{
    _is_truncated = truncated;
}

void _media_sample_impl::codec_name_set(const std::string& codec_name)
{
    _codec_name = codec_name;
}

const int _media_sample_impl::stream_number() const
{
    return _stream_number;
}

void _media_sample_impl::stream_number_set(int stream_number)
{
    _stream_number = stream_number;
}

const uint8_t* _media_sample_impl::data() const
{
    throw std::exception();
}

const std::chrono::microseconds _media_sample_impl::presentation_time() const
{
    return _presentation_time;
}

bool _media_sample_impl::is_complete_sample() const
{
    return _is_complete_sample;
}

bool _media_sample_impl::is_truncated() const
{
    return _is_truncated;
}

const std::string _media_sample_impl::codec_name() const
{
    return _codec_name;
}

_media_sample_impl::~_media_sample_impl()
{

}

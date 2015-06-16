#include "details/_media_sample_impl.hpp"

using namespace uvxx::rtsp;
using namespace uvxx::rtsp::details;

_media_sample_impl::_media_sample_impl()
{
    
}

const size_t _media_sample_impl::size() const
{
    return _size;
}

void uvxx::rtsp::details::_media_sample_impl::size_set(size_t size)
{
    _size = size;
}

void uvxx::rtsp::details::_media_sample_impl::presentation_time_set(std::chrono::microseconds presentation_time)
{
    _presentation_time = presentation_time;
}

void uvxx::rtsp::details::_media_sample_impl::is_complete_sample_set(bool complete_sample)
{
    _is_complete_sample = complete_sample;
}

void uvxx::rtsp::details::_media_sample_impl::is_truncated_set(bool truncated)
{
    _is_truncated = truncated;
}

void uvxx::rtsp::details::_media_sample_impl::codec_name_set(const std::string& codec_name)
{
    _codec_name = codec_name;
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

#include "details/_media_sample_impl.hpp"

using namespace uvxx::io;
using namespace uvxx::rtsp;
using namespace uvxx::rtsp::details;

_media_sample_impl::_media_sample_impl() :
                        _stream_number(0),
                        _size(0),
                        _is_truncated(false),
                        _presentation_time(0)
{
    
}

size_t _media_sample_impl::size() const
{
    return _size;
}

void _media_sample_impl::size_set(size_t size)
{
    if (size > _buffer.size())
    {
        throw std::exception();
    }

   _size = size;
}

void _media_sample_impl::presentation_time_set(std::chrono::microseconds presentation_time)
{
    _presentation_time = presentation_time;
}

void _media_sample_impl::codec_name_set(const std::string& codec_name)
{
    _codec_name = codec_name;
}

int _media_sample_impl::stream_number() const
{
    return _stream_number;
}

void _media_sample_impl::stream_number_set(int stream_number)
{
    _stream_number = stream_number;
}

const uint8_t* _media_sample_impl::data() const
{
    return _buffer.data();
}

std::chrono::microseconds _media_sample_impl::presentation_time() const
{
    return _presentation_time;
}

bool _media_sample_impl::is_truncated() const
{
    return _is_truncated;
}

std::string _media_sample_impl::codec_name() const
{
    return _codec_name;
}

_media_sample_impl::~_media_sample_impl()
{

}

size_t _media_sample_impl::capacity()
{
    return _buffer.size();
}

void _media_sample_impl::capacity_set(size_t capacity)
{
    _buffer.resize(capacity);
}

void _media_sample_impl::is_truncated_set(bool truncated)
{
    _is_truncated = truncated;
}

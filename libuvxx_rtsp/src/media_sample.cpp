#include "media_sample.hpp"
#include "details/_media_sample_impl.hpp"

using namespace uvxx::rtsp;
using namespace details;

media_sample::media_sample()
{
    __media_sample_impl = std::make_shared<_media_sample_impl>();
}

const size_t media_sample::size() const
{
    return __media_sample_impl->size();
}

const uint8_t* media_sample::data() const
{
    return __media_sample_impl->data();
}

const std::chrono::microseconds media_sample::presentation_time() const
{
    return __media_sample_impl->presentation_time();
}

bool media_sample::is_complete_sample() const
{
    return __media_sample_impl->is_complete_sample();
}

bool media_sample::is_truncated() const
{
    return __media_sample_impl->is_truncated();
}

const std::string media_sample::codec_name() const
{
    return __media_sample_impl->codec_name();
}

const int uvxx::rtsp::media_sample::stream_number() const
{
    return __media_sample_impl->stream_number();
}

void uvxx::rtsp::media_sample::size_set(size_t size) const
{
    __media_sample_impl->size_set(size);
}

const size_t uvxx::rtsp::media_sample::capacity() const
{
    return __media_sample_impl->capacity();
}

void uvxx::rtsp::media_sample::capacity_set(size_t size) const
{
    __media_sample_impl->capacity_set(size);
}

void uvxx::rtsp::media_sample::set_attribute(const std::string& attribute_name, const uvxx::io::memory_buffer& buffer) const
{
    __media_sample_impl->set_attribute(attribute_name, buffer);
}

uvxx::io::memory_buffer uvxx::rtsp::media_sample::get_attribute(const std::string& attribute_name) const
{
    return __media_sample_impl->get_attribute(attribute_name);
}

void uvxx::rtsp::media_sample::codec_name_set(const std::string& codec_name)
{
    __media_sample_impl->codec_name_set(codec_name);
}

void uvxx::rtsp::media_sample::presentation_time_set(std::chrono::microseconds presentation_time) const
{
    __media_sample_impl->presentation_time_set(presentation_time);
}

void uvxx::rtsp::media_sample::is_truncated_set(bool truncated) const
{
    __media_sample_impl->is_truncated_set(truncated);
}

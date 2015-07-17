#include "media_sample.hpp"
#include "details/_media_sample_impl.hpp"

using namespace uvxx::io;
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

bool media_sample::is_truncated() const
{
    return __media_sample_impl->is_truncated();
}

const std::string media_sample::codec_name() const
{
    return __media_sample_impl->codec_name();
}

const int media_sample::stream_number() const
{
    return __media_sample_impl->stream_number();
}

void media_sample::size_set(size_t size) const
{
    __media_sample_impl->size_set(size);
}

const size_t media_sample::capacity() const
{
    return __media_sample_impl->capacity();
}

void media_sample::capacity_set(size_t size) const
{
    __media_sample_impl->capacity_set(size);
}

void media_sample::attribute_blob_set(const std::string& attribute_name, const memory_buffer& buffer) const
{
    __media_sample_impl->attribute_set(attribute_name, buffer);
}

memory_buffer media_sample::attribute_blob_get(const std::string& attribute_name) const
{
    return __media_sample_impl->attribute_get(attribute_name);
}

void media_sample::codec_name_set(const std::string& codec_name)
{
    __media_sample_impl->codec_name_set(codec_name);
}

void media_sample::presentation_time_set(std::chrono::microseconds presentation_time) const
{
    __media_sample_impl->presentation_time_set(presentation_time);
}

void media_sample::is_truncated_set(bool truncated) const
{
    __media_sample_impl->is_truncated_set(truncated);
}

void media_sample::clear_attributes()
{
    __media_sample_impl->clear_attributes();
}

void media_sample::stream_number_set(int stream_number)
{
    __media_sample_impl->stream_number_set(stream_number);
}

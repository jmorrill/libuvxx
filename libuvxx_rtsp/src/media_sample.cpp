#include "media_sample.hpp"
#include "details/_media_sample_impl.hpp"

using namespace uvxx::io;
using namespace uvxx::rtsp;
using namespace details;

media_sample::media_sample() : media_sample(std::make_shared<_media_sample_impl>())
{
}

media_sample::media_sample(const details::_media_sample_impl_ptr& impl) : 
    media_attributes(impl), __media_sample_impl(impl)
{
}

size_t media_sample::size() const
{
    return __media_sample_impl->size();
}

const uint8_t* media_sample::data() const
{
    return __media_sample_impl->data();
}

std::chrono::microseconds media_sample::presentation_time() const
{
    return __media_sample_impl->presentation_time();
}

bool media_sample::is_truncated() const
{
    return __media_sample_impl->is_truncated();
}

std::string media_sample::codec_name() const
{
    return __media_sample_impl->codec_name();
}

media_sample::media_sample(media_sample&& rhs)
{
    *this = std::move(rhs);
}

media_sample& media_sample::operator=(media_sample&& rhs)
{
    if (this != &rhs)
    {
        __media_sample_impl = std::move(__media_sample_impl);
    }

    return *this;
}

bool media_sample::operator=(std::nullptr_t)
{
    __media_sample_impl = nullptr;

    return true;
}

bool media_sample::operator==(std::nullptr_t)
{
    return __media_sample_impl == nullptr;
}

bool media_sample::operator!=(std::nullptr_t)
{
    return __media_sample_impl != nullptr;
}

int media_sample::stream_number() const
{
    return __media_sample_impl->stream_number();
}

void media_sample::size_set(size_t size)
{
    __media_sample_impl->size_set(size);
}

size_t media_sample::capacity() const
{
    return __media_sample_impl->capacity();
}

void media_sample::capacity_set(size_t size)
{
    __media_sample_impl->capacity_set(size);
}

void media_sample::codec_name_set(const std::string& codec_name)
{
    __media_sample_impl->codec_name_set(codec_name);
}

void media_sample::presentation_time_set(std::chrono::microseconds presentation_time)
{
    __media_sample_impl->presentation_time_set(presentation_time);
}

void media_sample::is_truncated_set(bool truncated)
{
    __media_sample_impl->is_truncated_set(truncated);
}

void media_sample::stream_number_set(int stream_number)
{
    __media_sample_impl->stream_number_set(stream_number);
}

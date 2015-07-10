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

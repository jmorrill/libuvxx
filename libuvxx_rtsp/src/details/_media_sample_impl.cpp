#include "details/_media_sample_impl.hpp"

using namespace uvxx::rtsp;
using namespace uvxx::rtsp::details;

_media_sample_impl::_media_sample_impl()
{
    throw std::exception();
}

const size_t _media_sample_impl::size() const
{
    throw std::exception();
}

const uint8_t* _media_sample_impl::data() const
{
    throw std::exception();
}

const std::chrono::microseconds _media_sample_impl::presentation_time() const
{
    throw std::exception();
}

bool _media_sample_impl::complete_sample() const
{
    throw std::exception();
}

bool _media_sample_impl::truncated() const
{
    throw std::exception();
}

const std::string _media_sample_impl::codec_name() const
{
    throw std::exception();
}

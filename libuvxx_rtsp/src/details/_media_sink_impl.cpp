#include "details/_media_sink_impl.hpp"

using namespace uvxx::rtsp::details;

_media_sink_impl::_media_sink_impl(const _media_subsession_impl& media_subsession, 
                                   const std::shared_ptr<UsageEnvironment>& live_usage_environment) :
    _live_usage_environment(live_usage_environment),
    _media_subsession(media_subsession)
{
    __live_media_sink = std::shared_ptr<_live_media_sink>(new _live_media_sink(*(live_usage_environment.get())),
    /* deleter */
    [](MediaSink* media_sink)
    {
        Medium::close(media_sink);
    });

    __live_media_sink->on_frame_callback_set(std::bind(&_media_sink_impl::on_frame, this, std::placeholders::_1));
}

void _media_sink_impl::on_frame(const _live_media_sink_frame& frame)
{

}

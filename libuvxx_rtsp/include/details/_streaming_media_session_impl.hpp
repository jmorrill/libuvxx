#pragma once
#include "uvxx.hpp"
#include "media_session.hpp"

#include <memory>
#include <vector>
#include <string>

namespace uvxx { namespace rtsp
{
    class media_subsession;
}}

namespace uvxx { namespace rtsp { namespace details 
{
    class _live_media_session_sink;
    
    class _streaming_media_session_impl : uvxx::event_dispatcher_object
    {
    public:
        _streaming_media_session_impl(const std::vector<media_subsession>& subsessions);

        _streaming_media_session_impl(){}

        virtual ~_streaming_media_session_impl();

    private:
        static void on_rtcp_bye(void* client_data);

    private:
        std::vector<media_subsession> _subsessions;

    };
}}}
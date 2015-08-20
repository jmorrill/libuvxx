#pragma once
#include <functional>

#include "event_dispatcher_object.hpp"
#include "rtsp_misc.hpp"
#include "stream_statistics.hpp"
#include "media_descriptor.hpp"
#include "event_dispatcher_object_base.hpp"

namespace uvxx { namespace rtsp { namespace details
{
    /* implementation forward */
    class _rtsp_client_impl;

    using _rtsp_client_impl_ptr = std::shared_ptr<_rtsp_client_impl>;
}}}

namespace uvxx { namespace rtsp 
{
    class media_session;

    class media_subsession;

    class media_sample;

    using read_sample_delegate = std::function<void(const media_sample&)>;

    using stream_closed_delegate = std::function<void(int stream_number)>;
    
    class rtsp_client : public event_dispatcher_object_base<details::_rtsp_client_impl>
    {
    public:
        rtsp_client();

        rtsp_client(const rtsp_client& client) = default;

    public:
        pplx::task<void> open(const std::string& url) const;

        pplx::task<void> play() const;

        pplx::task<void> play(std::vector<media_subsession> media_subsessions) const;

        void on_sample_set(read_sample_delegate callback) const;
        
        void on_stream_closed_set(stream_closed_delegate callback) const;

        void read_stream_sample() const;

        media_session session() const;

        void credentials_set(const std::string& username, const std::string& pass);

        std::string username() const;

        std::string password() const;

        transport_protocol protocol() const;

        void protocol_set(transport_protocol protocol);
        
        void timeout_set(std::chrono::milliseconds timeout);

        std::chrono::milliseconds timeout() const;

        stream_statistics stream_statistics_get(int stream_id) const;

        media_descriptor media_descriptor_get();
    };
}}
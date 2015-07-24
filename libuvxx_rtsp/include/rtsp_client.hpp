#pragma once
#include <functional>

#include "event_dispatcher_object.hpp"
#include "rtsp_misc.hpp"

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

    class rtsp_client : public event_dispatcher_object
    {
    public:
        rtsp_client();

        rtsp_client(const rtsp_client& client) : event_dispatcher_object()
        {
           // *this = rtsp_client;
            __rtsp_client_imp = client.__rtsp_client_imp;
        }

        virtual rtsp_client& operator=(const rtsp_client&) = default;

        rtsp_client(rtsp_client&& rhs);

        virtual rtsp_client& operator=(rtsp_client&& rhs);

    public:
        pplx::task<void> open(const std::string& url) const;

        pplx::task<void> play() const;

        pplx::task<void> play(std::vector<media_subsession> media_subsessions) const;

        void on_sample_set(read_sample_delegate callback) const;

        void read_stream_sample() const;

        media_session session() const;

        void credentials_set(const std::string& username, const std::string& pass);

        std::string username() const;

        std::string password() const;

        transport_protocol protocol() const;

        void protocol_set(transport_protocol protocol);

    private:
        details::_rtsp_client_impl_ptr __rtsp_client_imp;
    };
}}
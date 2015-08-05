#pragma once
#include <memory>
#include "rtsp_client.hpp"
#include "event_dispatcher_timer.hpp"
#include "_live_authenticator.hpp"
#include "rtsp_misc.hpp"
#include "media_session.hpp"
#include "_live_common.hpp"

class RTSPClient;

namespace uvxx { namespace rtsp
{
    class media_session;

    class media_subsession;
}}

namespace uvxx { namespace rtsp { namespace details 
{
    class _rtsp_client_impl;

    class _live_rtsp_client;

    class _streaming_media_session_impl;

    using _streaming_media_session_impl_ptr = std::shared_ptr<_streaming_media_session_impl>;

    using _live_rtsp_client_ptr = std::shared_ptr<_live_rtsp_client>;

    using _rtsp_client_impl_ptr = std::shared_ptr<_rtsp_client_impl>;
   
    class _rtsp_client_impl : public event_dispatcher_object
    {
    public:
        _rtsp_client_impl();

        ~_rtsp_client_impl();

        _rtsp_client_impl(const _rtsp_client_impl&) = delete;

        _rtsp_client_impl& operator=(const _rtsp_client_impl&) = delete;

    public:
        pplx::task<void> open(const std::string& url);

        pplx::task<void> play(std::vector<media_subsession> subsessions);

        void on_sample_callback_set(read_sample_delegate callback);

        void on_stream_closed_set(stream_closed_delegate callback);

        void read_stream_sample();

        media_session session();

        std::string username() const;

        void credentials_set(const std::string& username, const std::string& password);

        std::string password() const;

        transport_protocol protocol() const;

        void protocol_set(transport_protocol protocol);

        void timeout_set(std::chrono::milliseconds timeout);

        std::chrono::milliseconds timeout();

        stream_statistics stream_statistics_get(int stream_id) const;

    private:
        pplx::task<void> setup(const std::shared_ptr<std::vector<media_subsession>>& subsessions);

        void on_timeout_timer_tick(uvxx::event_dispatcher_timer* sender);

    private:
        static void describe_callback(RTSPClient* live_rtsp_client, int result_code, char* result_string);

        static void setup_callback(RTSPClient* live_rtsp_client, int result_code, char* result_string);

        static void play_callback(RTSPClient* live_rtsp_client, int result_code, char* result_string);

    private:
        unsigned _last_rtsp_command_id;

        pplx::task_completion_event<void> _current_event;

        pplx::task_completion_event<void> _describe_event;
        
        pplx::task_completion_event<void> _setup_event;
        
        pplx::task_completion_event<void> _play_event;

        media_session _session;

        _live_rtsp_client_ptr _live_client;

        _streaming_media_session_impl_ptr _streaming_session;

        _live_authenticator _authenticator;

        transport_protocol _protocol;

        std::string _username;
        
        std::string _password;

        read_sample_delegate _read_sample_delegate;
        
        stream_closed_delegate _stream_closed_delegate;

        event_dispatcher_timer _timeout_timer;
        
        std::chrono::milliseconds _timeout;

        _usage_environment_ptr _usage_environment;
    };

    using _rtsp_client_impl_ptr = std::shared_ptr<_rtsp_client_impl>;
}}}
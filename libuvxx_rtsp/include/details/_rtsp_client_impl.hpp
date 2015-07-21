#pragma once
#include <memory>
#include "rtsp_client.hpp"
#include "_media_session_impl.hpp"
#include "streaming_media_session.hpp"
#include "_live_authenticator.hpp"
#include "rtsp_misc.hpp"
#include "media_session.hpp"

class UsageEnvironment;

class RTSPClient;

namespace uvxx { namespace rtsp
{
    class media_session;

    class media_subsession;
}}

namespace uvxx { namespace rtsp { namespace details 
{
    class _uvxx_task_scheduler;

    class _rtsp_client_impl;

    class _live_rtsp_client;

    using _live_rtsp_client_ptr = std::shared_ptr<_live_rtsp_client>;

    using _usage_environment_ptr = std::shared_ptr<UsageEnvironment>;

    using _rtsp_client_impl_ptr = std::shared_ptr<_rtsp_client_impl>;
   
    class _rtsp_client_impl : public event_dispatcher_object
    {
    public:
        _rtsp_client_impl();

        ~_rtsp_client_impl();

    public:
        pplx::task<void> open(const std::string& url);

        pplx::task<void> play(std::vector<media_subsession> subsessions);

		void on_sample_callback_set(read_sample_delegate callback);

        void read_stream_sample();

        media_session session();

        std::string username() const;

        void credentials_set(const std::string& username, const std::string& password);

        std::string password() const;

        transport_protocol protocol() const;

        void protocol_set(transport_protocol protocol);

    private:
        pplx::task<void> setup(const std::shared_ptr<std::vector<media_subsession>>& subsessions);

    private:
        static void describe_callback(RTSPClient* live_rtsp_client, int result_code, char* result_string);

        static void setup_callback(RTSPClient* live_rtsp_client, int result_code, char* result_string);

        static void play_callback(RTSPClient* live_rtsp_client, int result_code, char* result_string);

    private:
        unsigned _last_rtsp_command_id;

        pplx::task_completion_event<void> _describe_event;
	    
        pplx::task_completion_event<void> _setup_event;
	    
        pplx::task_completion_event<void> _play_event;

        media_session _session;

        _usage_environment_ptr _usage_environment;

        _live_rtsp_client_ptr _live_client;

        _uvxx_task_scheduler* _task_scheduler;

        media_subsession _current_media_subsession_setup;

        streaming_media_session _streaming_session;

        _live_authenticator _authenticator;

        transport_protocol _protocol;

        std::string _username;
        
        std::string _password;

		read_sample_delegate _read_sample_delegate;
    };

    using _rtsp_client_impl_ptr = std::shared_ptr<_rtsp_client_impl>;
}}}
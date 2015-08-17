#pragma once
#include <memory>
#include <functional>
#include "RTSPServerSupportingHTTPStreaming.hh"
#include <pplx/pplxtasks.h>
#include "_live_server_media_session.hpp"

namespace uvxx { namespace rtsp { namespace details 
{
    using _lookup_media_session_delegate = std::function<pplx::task<_live_server_media_session*>(const std::string& stream_name)>;

    using _server_media_session_callback_delegate = std::function<void(_live_server_media_session*)>;

    class _live_rtsp_server : public RTSPServerSupportingHTTPStreaming
    {
    public:
        _live_rtsp_server(uint16_t port);

        _live_rtsp_server(const _live_rtsp_server&) = delete;

        _live_rtsp_server& operator=(const _live_rtsp_server&) = delete;
    
        virtual ~_live_rtsp_server() override;

    public:
        void set_on_lookup_media_session(_lookup_media_session_delegate callback);

        bool allow_streaming_rtp_over_tcp();

    protected:
        virtual ServerMediaSession* lookupServerMediaSession(char const* stream_name, Boolean is_first_lookup_in_session = true) override;

        virtual uvxx::pplx::task<_live_server_media_session*> begin_lookup_server_media_session(const std::string& stream_name, bool is_first_lookup_in_session, uint32_t client_session_id);

        virtual ClientSession* createNewClientSession(u_int32_t session_id) override;

    protected:
        virtual ClientConnection* createNewClientConnection(int client_socket, struct sockaddr_in client_addr) override;

        class _live_rtsp_client_connection;

        class _live_rtsp_client_session : public RTSPClientSession
        {
        public:
            _live_rtsp_client_session(_live_rtsp_server& our_server, u_int32_t session_id);

            virtual ~_live_rtsp_client_session() = default;

        public:
            bool stream_after_setup_get();

            void note_liveness();

            virtual uvxx::pplx::task<void> begin_handle_setup(_live_rtsp_client_connection* our_client_connection, const std::string& url_pre_suffix, const std::string& url_suffix, const std::string& full_request_str);

            virtual uvxx::pplx::task<void> handle_cmd_setup(_live_rtsp_client_connection* our_client_connection, _live_server_media_session* sms, const std::string& cseq, const std::string& url_pre_suffix, const std::string& url_suffix, const std::string& full_request_str);

            virtual void handle_cmd_within_session(RTSPClientConnection* ourClientConnection, char const* cmdName, char const* urlPreSuffix, char const* urlSuffix, char const* fullRequestStr);

        protected:
            _live_rtsp_server& _our_server;

        };

        class _live_rtsp_client_connection : public RTSPClientConnectionSupportingHTTPStreaming
        {
        public:
            _live_rtsp_client_connection(_live_rtsp_server& our_server, int client_socket, struct sockaddr_in client_addr);

            virtual ~_live_rtsp_client_connection();

            virtual void handleRequestBytes(int newBytesRead) override;

            virtual void handleCmd_unsupportedTransport() override;

            virtual uvxx::pplx::task<void> begin_handle_describe(const std::string& url_pre_suffix, const std::string& url_suffix, const std::string& full_request_str);

            int client_output_socket();

            int client_input_socket();

            sockaddr_in client_addr();

            const char* current_cseq();

            unsigned char* response_buffer();

            size_t response_buffer_size();

            virtual void handleCmd_notFound() override;

            virtual void handleCmd_bad() override;

        private:
            _live_rtsp_server& __live_rtsp_server;

        };

    private:
        static int setup_socket(uint16_t port);

    private:
        _lookup_media_session_delegate __lookup_media_session_delegate;

    };

    using _live_rtsp_server_ptr = std::shared_ptr<_live_rtsp_server>;
}}}
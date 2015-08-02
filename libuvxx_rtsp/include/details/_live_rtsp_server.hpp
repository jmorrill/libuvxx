#pragma once
#include <memory>
#include <functional>

#include "RTSPServerSupportingHTTPStreaming.hh"

class RTSPServerSupportingHTTPStreaming;

namespace uvxx { namespace rtsp { namespace details 
{
    using _usage_environment_ptr = std::shared_ptr<UsageEnvironment>;

    using _lookup_media_session_delegate = std::function<ServerMediaSession*(const std::string& stream_name)>;

    class _live_rtsp_server : public RTSPServerSupportingHTTPStreaming
    {
    public:
        _live_rtsp_server(const _usage_environment_ptr& environment, int port);

        _live_rtsp_server(const _live_rtsp_server&) = delete;

        _live_rtsp_server& operator=(const _live_rtsp_server&) = delete;
    
        virtual ~_live_rtsp_server() override;

    public:
        void set_on_lookup_media_session(_lookup_media_session_delegate callback);

    protected:
        virtual ServerMediaSession* lookupServerMediaSession(char const* stream_name, Boolean is_first_lookup_in_session) override;

    private:
        static int setup_socket(const _usage_environment_ptr& environment, int port);

        _usage_environment_ptr _usage_environment;

        _lookup_media_session_delegate __lookup_media_session_delegate;
    };

    using _live_rtsp_server_ptr = std::shared_ptr<_live_rtsp_server>;
}}}
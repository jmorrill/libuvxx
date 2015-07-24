#pragma once
#include <memory>
#include "RTSPClient.hh"

namespace uvxx { namespace rtsp { namespace details 
{
    using _usage_environment_ptr = std::shared_ptr<UsageEnvironment>;

    class _live_rtsp_client : public RTSPClient
    {
    public:
        _live_rtsp_client(const _usage_environment_ptr& environment,
                          char const* rtsp_url,
                          void* context,
                          int verbosity_level = 0,
                          char const* application_name = nullptr,
                          portNumBits tunnel_over_http_port_number = 0);

        _live_rtsp_client(const _live_rtsp_client&) = delete;

        _live_rtsp_client& operator=(const _live_rtsp_client&) = delete;

        void* context();

        virtual ~_live_rtsp_client();

    private:
        void* _context;

        _usage_environment_ptr _usage_environment;
    };

    using _live_rtsp_client_ptr = std::shared_ptr<_live_rtsp_client>;
}}}
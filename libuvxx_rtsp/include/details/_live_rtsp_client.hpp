#pragma once
#include "RTSPClient.hh"
#include <memory>

namespace uvxx { namespace rtsp { namespace details 
{
    class _live_rtsp_client : public RTSPClient
    {
    public:
        _live_rtsp_client(UsageEnvironment& environment,
                          char const* rtsp_url,
                          void* context,
                          int verbosity_level = 0,
                          char const* application_name = nullptr,
                          portNumBits tunnel_over_http_port_number = 0);

        void* context_get();

        virtual ~_live_rtsp_client();

    private:
        void* _context;
    };

    using _live_rtsp_client_ptr = std::shared_ptr<_live_rtsp_client>;

}}}
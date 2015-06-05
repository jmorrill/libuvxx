#pragma once
#include "RTSPClient.hh"

namespace uvxx { namespace uvxx_rtsp { namespace details 
{
    class _live_rtsp_client : public RTSPClient
    {
    public:
        _live_rtsp_client(UsageEnvironment& env,
                          char const* rtspURL,
                          void* pContext,
                          int verbosityLevel = 0,
                          char const* applicationName = nullptr,
                          portNumBits tunnelOverHTTPPortNum = 0);

        void* get_context();

        virtual ~_live_rtsp_client();

    private:
        void* _context;
    };
}}}
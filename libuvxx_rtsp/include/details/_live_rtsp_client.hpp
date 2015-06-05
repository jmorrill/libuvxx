#pragma once
#include "RTSPClient.hh"

namespace uvxx { namespace rtsp { namespace details 
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

        void* context_get();

        virtual ~_live_rtsp_client();

    private:
        void* _context;
    };
}}}
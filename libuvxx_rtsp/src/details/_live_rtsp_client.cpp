#include "details/_live_rtsp_client.hpp"

using namespace uvxx::uvxx_rtsp::details;

uvxx::uvxx_rtsp::details::_live_rtsp_client::_live_rtsp_client(UsageEnvironment& env, 
                                     char const* rtspURL,
                                     void* pContext,
			                         int verbosityLevel,
			                         char const* applicationName,
			                         portNumBits tunnelOverHTTPPortNum) :
    RTSPClient(env, rtspURL, verbosityLevel, applicationName, tunnelOverHTTPPortNum, -1)
{
    
}

uvxx::uvxx_rtsp::details::_live_rtsp_client::~_live_rtsp_client(void)
{
    printf("_live_rtsp_client deleted\n");
}
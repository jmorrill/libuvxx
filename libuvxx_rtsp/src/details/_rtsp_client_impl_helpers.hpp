#pragma once


template <int code>
struct result_code_constant
{
    static int value()
    {
        return code;
    }
};

#define CAST_RTSP_CLIENT(live_rtsp_client)static_cast<uvxx::rtsp::details::_rtsp_client_impl*>(static_cast<uvxx::rtsp::details::_live_rtsp_client*>(live_rtsp_client)->context());

#define SET_RTSP_EXCEPTION(code, message, task_event)\
if(!code)\
{\
    task_event.set_exception(rtsp_exception(message ? message : ""));\
}\
else if(code < 0)\
{\
    task_event.set_exception(rtsp_network_exception(std::abs(code), message ? message : ""));\
}\
else\
{\
    task_event.set_exception(rtsp_transport_exception(code, message ? message : ""));\
}\

#define BEGIN_CALLBACK(live_rtsp_client, rtsp_client, async_event, result_code, rs)\
auto rtsp_client = CAST_RTSP_CLIENT(live_rtsp_client)\
\
rtsp_client->_timeout_timer.stop();\
\
rtsp_client->_last_rtsp_command_id = 0;\
\
auto resultstring = std::unique_ptr<char[]>(rs);\
\
if (result_code)\
{\
    SET_RTSP_EXCEPTION(result_code, result_string, rtsp_client->async_event );\
\
    return;\
}

#define END_CALLBACK(rtsp_client, async_event)\
rtsp_client->async_event.set()


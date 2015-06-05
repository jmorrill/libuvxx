#include "details/_rtsp_client_impl.hpp"

using namespace uvxx::uvxx_rtsp::details;

enum class _rtsp_handler_type
{
    Describe = 1,
    Options,
    Setup,
    Play,
    Pause
};

class _rtsp_response_handler
{

public:
    _rtsp_response_handler()
    {

    }
private:
    static void response_handler(RTSPClient* rtsp_client, int result_code, char* result_string)
    {
        
    }
};

void _rtsp_client_impl::continueAfterDESCRIBE(RTSPClient* live_rtsp_client, int result_code, char* result_string) 
{
    auto client = static_cast<_rtsp_client_impl*>(static_cast<_live_rtsp_client*>(live_rtsp_client)->get_context());

    client->_describe_event.set(result_code);
        
    if (result_code != 0)
    {
        delete[] result_string;

        client->_describe_event.set_exception(std::exception("failed to get a SDP description"));

        return;
    }
    
    // Create a media session object from this SDP description:
    auto session = MediaSession::createNew(*client->_usage_environment, result_string);
    delete[] result_string; // because we don't need it anymore

    if (session == nullptr)
    {
        client->_describe_event.set_exception(std::exception("failed to create a MediaSession object from the SDP description"));
        return;
    }
    else if (!session->hasSubsessions())
    {
        client->_describe_event.set_exception(std::exception("This session has no media subsessions"));
        return;
    }

    {
        client->_session.set_media_session(client->_usage_environment, session);
    }
}

_rtsp_client_impl::_rtsp_client_impl()
{
    _task_scheduler = _uvxx_task_scheduler::createNew();

    _usage_environment = std::shared_ptr<UsageEnvironment>(BasicUsageEnvironment::createNew(*(_task_scheduler)),
    [](UsageEnvironment* environment)
    {
        auto& task_scheduler = environment->taskScheduler();
        delete &task_scheduler;
        bool success = environment->reclaim();
    });
}

uvxx::pplx::task<void> uvxx::uvxx_rtsp::details::_rtsp_client_impl::open(const std::string& url)
{
    _live_client = std::shared_ptr<_live_rtsp_client>(new _live_rtsp_client(*_usage_environment, url.c_str(), this, 0),
        [](_live_rtsp_client* client)
        {
            Medium::close(client);
        });

    _describe_event = uvxx::pplx::task_completion_event<int>();
    
    auto seq = _live_client->sendDescribeCommand(continueAfterDESCRIBE);

    return uvxx::pplx::create_task(_describe_event).then([this](int result_code)
    {
        printf("result code");
        this->dispatcher().exit_all_frames();
    });
}


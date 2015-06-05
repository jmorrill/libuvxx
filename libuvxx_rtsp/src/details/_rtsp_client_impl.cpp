#include "details/_rtsp_client_impl.hpp"

using namespace uvxx::rtsp::details;

void _rtsp_client_impl::describe_callback(RTSPClient* live_rtsp_client, int result_code, char* result_string) 
{
    auto client = static_cast<_rtsp_client_impl*>(static_cast<_live_rtsp_client*>(live_rtsp_client)->context_get());
        
    if (result_code != 0)
    {
        delete[] result_string;

        client->_describe_event.set_exception(std::exception("failed to get a SDP description"));

        return;
    }
    
    // Create a media session object from this SDP description:
    auto session = MediaSession::createNew(*client->_usage_environment, result_string);
    delete[] result_string;

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

    client->_describe_event.set(result_code);

    client->_session = std::make_shared<_media_session>();

    client->_session->set_media_session(client->_usage_environment, session);
}


void uvxx::rtsp::details::_rtsp_client_impl::setup_callback(RTSPClient* live_rtsp_client, int result_code, char* result_string)
{
    auto client = static_cast<_rtsp_client_impl*>(static_cast<_live_rtsp_client*>(live_rtsp_client)->context_get());

    client->_setup_event.set(result_code);
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
            assert(success);
        });
}

uvxx::rtsp::details::_rtsp_client_impl::~_rtsp_client_impl()
{

}

uvxx::pplx::task<void> uvxx::rtsp::details::_rtsp_client_impl::open(const std::string& url)
{
    _live_client = std::shared_ptr<_live_rtsp_client>(new _live_rtsp_client(*_usage_environment, url.c_str(), this, 0),
        [](_live_rtsp_client* client)
        {
            Medium::close(client);
        });

    _describe_event = uvxx::pplx::task_completion_event<int>();
    
    _live_client->sendDescribeCommand(describe_callback);

    return uvxx::pplx::create_task(_describe_event).then([this](int result_code)
    {
        printf("result code %d\n", result_code);
    });
}

std::shared_ptr<_media_session> uvxx::rtsp::details::_rtsp_client_impl::media_session_get()
{
    return _session;
}

uvxx::pplx::task<void> uvxx::rtsp::details::_rtsp_client_impl::play()
{
    auto index = std::make_shared<size_t>(0);

    return uvxx::pplx::create_iterative_task([=]() 
    {
        return uvxx::pplx::create_task([=]{}).
        then([=]
        {
            auto sub_index = *index;

            if (sub_index >= _session->subsessions().size())
            {
                throw std::exception("done iterating");
            }

            auto& sub = _session->subsessions().at(sub_index);

            sub->initiate();

            (*index)++;

            _setup_event = uvxx::pplx::task_completion_event<int>();

            _live_client->sendSetupCommand(*(sub)->live_media_subsession_get(), setup_callback);
        
        }).then([=]
        {
            return uvxx::pplx::create_task(_setup_event);
        }).then([=](int x)
        {
            printf("finished");
        });
    },uvxx::pplx::task_continuation_context::use_current())
    .then([](uvxx::pplx::task<void> iterativeTask)
    {
        try
        {
            iterativeTask.get();
        }
        catch (const uvxx::pplx::iterative_task_complete_exception&)
        {
        }
    });
}


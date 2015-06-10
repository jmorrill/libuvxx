#include "details/_rtsp_client_impl.hpp"
#include "media_session.hpp"

using namespace uvxx::pplx;
using namespace uvxx::rtsp::details;

#define GET_RTSP_CLIENT(live_rtsp_client)static_cast<uvxx::rtsp::details::_rtsp_client_impl*>(static_cast<uvxx::rtsp::details::_live_rtsp_client*>(live_rtsp_client)->context_get());

void _rtsp_client_impl::describe_callback(RTSPClient* live_rtsp_client, int result_code, char* result_string) 
{
    auto client_impl = GET_RTSP_CLIENT(live_rtsp_client);

    auto resultstring = std::unique_ptr<char[]>(result_string);
        
    if (result_code)
    {
        std::string exception_message = "failed to get a SDP description error: " + result_code;

        client_impl->_describe_event.set_exception(std::exception(exception_message.c_str()));

        return;
    }
    
    /* create a media session object from this SDP description */
    auto session = MediaSession::createNew(*client_impl->_usage_environment, resultstring.get());

    if (!session)
    {
        client_impl->_describe_event.set_exception(std::exception("failed to create a MediaSession object from the SDP description"));
        return;
    }
    else if (!session->hasSubsessions())
    {
        client_impl->_describe_event.set_exception(std::exception("This session has no media subsessions"));
        return;
    }

    client_impl->_describe_event.set(result_code);

    client_impl->_session = std::make_shared<_media_session_impl>();

    client_impl->_session->live_media_session_set(client_impl->_usage_environment, session);
}

void _rtsp_client_impl::setup_callback(RTSPClient* live_rtsp_client, int result_code, char* result_string)
{
    auto client_impl = GET_RTSP_CLIENT(live_rtsp_client);

    auto resultstring = std::unique_ptr<char[]>(result_string);

    auto& subsession = client_impl->_current_media_subsession_setup;

    auto live_subsession = subsession.__media_subsession->live_media_subsession_get();

    client_impl->_setup_event.set(result_code);
}


void _rtsp_client_impl::play_callback(RTSPClient* live_rtsp_client, int result_code, char* result_string)
{
    auto client_impl = GET_RTSP_CLIENT(live_rtsp_client);

    auto resultstring = std::unique_ptr<char[]>(result_string);

    client_impl->_setup_event.set(result_code);
}


_rtsp_client_impl::_rtsp_client_impl()
{
    _task_scheduler = _uvxx_task_scheduler::createNew();

    _usage_environment = _usage_environment_ptr(BasicUsageEnvironment::createNew(*(_task_scheduler)),
    /* deleter*/
    [](UsageEnvironment* environment)
    {
        auto& task_scheduler = environment->taskScheduler();

        delete &task_scheduler;

        bool memory_freed = environment->reclaim();

        assert(memory_freed);
    });
}

_rtsp_client_impl::~_rtsp_client_impl()
{

}

task<void> _rtsp_client_impl::open(const std::string& url)
{
    _live_client = _live_rtsp_client_ptr(new _live_rtsp_client(*_usage_environment, url.c_str(), this, 2),
    /* deleter */
    [](_live_rtsp_client* client)
    {
        Medium::close(client);
    });

    _describe_event = task_completion_event<int>();
    
    _live_client->sendDescribeCommand(describe_callback);

    return create_task(_describe_event).then([this](int result_code)
    {
        printf("result code %d\n", result_code);
    });
}

_media_session_impl_ptr _rtsp_client_impl::media_session_get()
{
    return _session;
}

uvxx::pplx::task<_streaming_media_session_impl_ptr> uvxx::rtsp::details::_rtsp_client_impl::play(const std::vector<media_subsession>& subsessions)
{
    auto current_index = std::make_shared<size_t>(0);
    
    return create_iterative_task([=]
    {
        return create_task([=]{}).then([=]
        {
            auto subsession_index = *current_index;

            if (subsession_index >= subsessions.size())
            {
                throw iterative_task_complete_exception();
            }

            auto& subsession = subsessions.at(subsession_index);
            
            subsession.__media_subsession->initiate();

            (*current_index)++;

            _setup_event = task_completion_event<int>();

            _live_client->sendSetupCommand(*(subsession.__media_subsession)->live_media_subsession_get(), 
                                           setup_callback);

            _current_media_subsession_setup = std::move(subsession);

        }).then([=]
        {
            return create_task(_setup_event);

        }).then([=](int result_code)
        {
            if (result_code)
            {
                std::string exception_message = "rtsp error " + result_code;

                throw std::exception(exception_message.c_str());
            }

            printf("finished play\n");
        });
    }, task_continuation_context::use_current())
    .then([=](task<void> iterativeTask)
    {
        _current_media_subsession_setup = nullptr;

        try
        {
            iterativeTask.get();
        }
        catch (const iterative_task_complete_exception&)
        {
        }

        (*current_index) = 0;

        return create_iterative_task([=]
        {
            return create_task([=]{}).then([=]
            {
                auto subsession_index = *current_index;

                if (subsession_index >= subsessions.size())
                {
                    throw iterative_task_complete_exception();
                }

                auto& subsession = subsessions.at(subsession_index);

                 (*current_index)++;

                _play_event = task_completion_event<int>();

                _live_client->sendPlayCommand(*(subsession.__media_subsession)->live_media_subsession_get(), 
                                               play_callback);
            });
        }, task_continuation_context::use_current());
    }).then([=](task<void> iterativeTask)
    {
        try
        {
            iterativeTask.get();
        }
        catch (const iterative_task_complete_exception&)
        {
        }

        return std::make_shared<_streaming_media_session_impl>(subsessions);
    });
}



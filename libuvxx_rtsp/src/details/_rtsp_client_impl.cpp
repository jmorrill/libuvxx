#include "details/_rtsp_client_impl.hpp"
#include "media_session.hpp"
#include "rtsp_exceptions.hpp"

using namespace uvxx::pplx;
using namespace uvxx::rtsp;
using namespace uvxx::rtsp::details;

#define GET_RTSP_CLIENT(live_rtsp_client)static_cast<uvxx::rtsp::details::_rtsp_client_impl*>(static_cast<uvxx::rtsp::details::_live_rtsp_client*>(live_rtsp_client)->context_get());

#define RTSP_EXCEPTION(code, message)((code > 0) ? rtsp_transport_exception(code, message) : rtsp_network_exception(code, message))

void _rtsp_client_impl::describe_callback(RTSPClient* live_rtsp_client, int result_code, char* result_string) 
{
    auto client_impl = GET_RTSP_CLIENT(live_rtsp_client);

    auto resultstring = std::unique_ptr<char[]>(result_string);

    auto& describe_event = client_impl->_describe_event;
        
    if (result_code)
    {
        std::string exception_message = "failed to get a SDP description error: " + result_code;

        describe_event.set_exception(rtsp_transport_exception(result_code, exception_message));

        return;
    }
    
    /* create a media session object from this SDP description */
    auto session = MediaSession::createNew(*client_impl->_usage_environment, resultstring.get());

    if (!session)
    {
        describe_event.set_exception(std::exception("failed to create a MediaSession object from the SDP description"));
        return;
    }
    else if (!session->hasSubsessions())
    {
        describe_event.set_exception(std::exception("This session has no media subsessions"));
        return;
    }

    describe_event.set(result_code);

    client_impl->_session = std::make_shared<_media_session_impl>();

    client_impl->_session->live_media_session_set(client_impl->_usage_environment, session);
}

void _rtsp_client_impl::setup_callback(RTSPClient* live_rtsp_client, int result_code, char* result_string)
{
    auto client_impl = GET_RTSP_CLIENT(live_rtsp_client);

    auto resultstring = std::unique_ptr<char[]>(result_string);

    auto& subsession = client_impl->_current_media_subsession_setup;

    auto live_subsession = subsession.__media_subsession->live_media_subsession_get();

    auto& setup_event = client_impl->_setup_event;

    setup_event.set(result_code);
}


void _rtsp_client_impl::play_callback(RTSPClient* live_rtsp_client, int result_code, char* result_string)
{
    auto client_impl = GET_RTSP_CLIENT(live_rtsp_client);

    auto resultstring = std::unique_ptr<char[]>(result_string);

    client_impl->_play_event.set(result_code);
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
    
    _authenticator.setUsernameAndPassword("admin","12345");

    _live_client->sendDescribeCommand(describe_callback, &_authenticator);

    return create_task(_describe_event).then([this](int result_code)
    {
        printf("result code %d\n", result_code);
    });
}

_media_session_impl_ptr _rtsp_client_impl::media_session_get()
{
    return _session;
}


uvxx::pplx::task<void> uvxx::rtsp::details::_rtsp_client_impl::setup(const std::shared_ptr<std::vector<media_subsession>>& subsessions_)
{
    auto current_index = std::make_shared<size_t>(0);

    return create_iterative_task([=]
    {
        verify_access();

        return create_task([=]
        {
            verify_access();

            auto& subsessions = *subsessions_.get();

            auto subsession_index = *current_index;

            if (subsession_index >= subsessions.size())
            {
                return task_from_exception<int>(iterative_task_complete_exception());
            }

            auto& subsession = subsessions.at(subsession_index);
            
            subsession.__media_subsession->initiate();

            (*current_index)++;

            _setup_event = task_completion_event<int>();

            _live_client->sendSetupCommand(*(subsession.__media_subsession)->live_media_subsession_get(), 
                                           setup_callback);

            _current_media_subsession_setup = subsession;

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
    });
}

uvxx::pplx::task<streaming_media_session> _rtsp_client_impl::play(std::vector<media_subsession> subsessions_)
{
    auto current_index = std::make_shared<size_t>(0);

    auto subsession_ptr = std::shared_ptr<std::vector<media_subsession>>(new std::vector<media_subsession>);

    *(subsession_ptr.get()) = std::move(subsessions_);

    return setup(subsession_ptr).then([=](task<void> iterativeTask)
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
            return create_task([=]
            {
                auto subsession_index = *current_index;

                auto& subsessions = *subsession_ptr.get();

                if (subsession_index >= subsessions.size())
                {
                    return task_from_exception<int>(iterative_task_complete_exception());
                }

                auto& subsession = subsessions.at(subsession_index);

                (*current_index)++;

                _play_event = task_completion_event<int>();

                _live_client->sendPlayCommand(*(subsession.__media_subsession)->live_media_subsession_get(), 
                                              play_callback);

                 return create_task(_play_event);
            }).then([=](int result_code)
            {
                if (result_code)
                {
                    std::string exception_message = "rtsp error " + result_code;

                    return task_from_exception<void>(std::exception(exception_message.c_str()));
                }

                printf("finished play\n");

                return task_from_result();
            });
        });
    }).then([=](task<void> iterativeTask)
    {
        try
        {
            iterativeTask.get();
        }
        catch (const iterative_task_complete_exception&)
        {
        }

        _streaming_session = streaming_media_session(std::make_shared<_streaming_media_session_impl>(_session, std::move(*subsession_ptr.get())));

        return _streaming_session;
    });
}



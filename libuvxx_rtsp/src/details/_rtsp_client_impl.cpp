#include "BasicUsageEnvironment.hh"

#include "uvxx.hpp"
#include "media_session.hpp"
#include "rtsp_exceptions.hpp"

#include "details/_uvxx_task_scheduler.hpp"
#include "details/_rtsp_client_impl.hpp"
#include "details/_streaming_media_session_impl.hpp"
#include "details/_live_rtsp_client.hpp"

using namespace uvxx;
using namespace uvxx::pplx;
using namespace uvxx::rtsp;
using namespace uvxx::rtsp::details;

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

void _rtsp_client_impl::describe_callback(RTSPClient* live_rtsp_client, int result_code, char* result_string) 
{
    auto client_impl = CAST_RTSP_CLIENT(live_rtsp_client);

    client_impl->_timeout_timer.stop();

    auto resultstring = std::unique_ptr<char[]>(result_string);

    auto& describe_event = client_impl->_describe_event;
        
    if (result_code)
    {
        SET_RTSP_EXCEPTION(result_code, result_string, describe_event);
        return;
    }
    
    /* create a media session object from this SDP description */
    auto session = MediaSession::createNew(*client_impl->_usage_environment, resultstring.get());

    if (!session)
    {
        SET_RTSP_EXCEPTION(result_code_constant<0>::value(), "failed to create a MediaSession from the SDP description", describe_event);
        return;
    }
    else if (!session->hasSubsessions())
    {
        SET_RTSP_EXCEPTION(result_code_constant<0>::value(), "failed to create a MediaSession from the SDP description", describe_event);
        return;
    }

    auto session_impl = std::make_shared<_media_session_impl>();

    session_impl->live_media_session_set(client_impl->_usage_environment, session);

    client_impl->_session = media_session(session_impl);

    describe_event.set();
}

void _rtsp_client_impl::setup_callback(RTSPClient* live_rtsp_client, int result_code, char* result_string)
{
    auto client_impl = CAST_RTSP_CLIENT(live_rtsp_client);

    client_impl->_timeout_timer.stop();

    auto resultstring = std::unique_ptr<char[]>(result_string);

    auto& setup_event = client_impl->_setup_event;

    if (result_code)
    {
        SET_RTSP_EXCEPTION(result_code, result_string, setup_event);

        return;
    }

    /*
    auto& subsession = client_impl->_current_media_subsession_setup;
    
    auto live_subsession = subsession.__media_subsession->live_media_subsession();
  
    unsigned const thresh = 1200000; // 1.2 seconds

    live_subsession->rtpSource()->setPacketReorderingThresholdTime(thresh);

    int socketNum = live_subsession->rtpSource()->RTPgs()->socketNum();

    unsigned curBufferSize = getReceiveBufferSize(live_subsession->rtpSource()->envir(), socketNum);
      
    unsigned newBufferSize = 200 * 1024;

    newBufferSize = setReceiveBufferTo(live_subsession->rtpSource()->envir(), socketNum, newBufferSize);
    */

    setup_event.set();
}

void _rtsp_client_impl::play_callback(RTSPClient* live_rtsp_client, int result_code, char* result_string)
{
    auto client_impl = CAST_RTSP_CLIENT(live_rtsp_client);

    client_impl->_timeout_timer.stop();

    auto resultstring = std::unique_ptr<char[]>(result_string);

    auto& play_event = client_impl->_play_event;

    if (result_code)
    {
        SET_RTSP_EXCEPTION(result_code, result_string, play_event);

        return;
    }

    client_impl->_play_event.set();
}

void _rtsp_client_impl::on_timeout_timer_tick(event_dispatcher_timer* /*sender*/)
{
    if (_live_client && _last_rtsp_command_id)
    {
        _live_client->changeResponseHandler(_last_rtsp_command_id, nullptr);

        _last_rtsp_command_id = 0;
    }

    _current_event.set_exception(rtsp_network_timeout("rtsp command timeout"));
}

_rtsp_client_impl::_rtsp_client_impl() : _last_rtsp_command_id(0),
                                         _task_scheduler(nullptr),
                                         _protocol(transport_protocol::udp),
                                         _timeout(10000)
{
    _timeout_timer.timeout_set(_timeout);

    /* Hook the tick event */
    _timeout_timer.tick_event() += std::bind(&_rtsp_client_impl::on_timeout_timer_tick, this, std::placeholders::_1);
 }

_rtsp_client_impl::~_rtsp_client_impl()
{
    if (_live_client && _last_rtsp_command_id)
    {
        _live_client->changeResponseHandler(_last_rtsp_command_id, nullptr);

        _last_rtsp_command_id = 0;
    }
}

task<void> _rtsp_client_impl::open(const std::string& url)
{
    verify_access();

    _streaming_session = nullptr;

    _task_scheduler = _uvxx_task_scheduler::createNew();

    _usage_environment = _usage_environment_ptr(BasicUsageEnvironment::createNew(*_task_scheduler),
    /* deleter*/
    [](UsageEnvironment* environment)
    {
        auto& task_scheduler = environment->taskScheduler();

        delete &task_scheduler;

        if(!environment->reclaim())
        {
            assert(false);
        }
    });
    
    _live_client = _live_rtsp_client_ptr(new _live_rtsp_client(_usage_environment, url.c_str(), this, 2),
    /* deleter */
    [](_live_rtsp_client* client)
    {
        Medium::close(client);
    });

    _describe_event = _current_event = task_completion_event<void>();
    
    _last_rtsp_command_id = _live_client->sendDescribeCommand(describe_callback, &_authenticator);

    _timeout_timer.start();

    return create_task(_describe_event).then([this]
    {
        printf("describe complete\n");
    });
}

media_session _rtsp_client_impl::session()
{
    verify_access();

    return _session;
}

task<void> _rtsp_client_impl::setup(const std::shared_ptr<std::vector<media_subsession>>& subsessions_)
{
    verify_access();

    return create_for_task<size_t>(0, subsessions_->size(), [=](size_t i)
    {
        auto& subsession = subsessions_->at(i);

        subsession.__media_subsession->initiate();

        _setup_event = _current_event = task_completion_event<void>();

        _last_rtsp_command_id = _live_client->sendSetupCommand(*(subsession.__media_subsession)->live_media_subsession(),
                                                               setup_callback,
                                                               false,
                                                               _protocol == transport_protocol::tcp ? true : false);

        _current_media_subsession_setup = subsession;

        _timeout_timer.start();

        return create_task(_setup_event);
    });
}


void _rtsp_client_impl::protocol_set(transport_protocol protocol)
{
    verify_access();

    _protocol = protocol;
}

void _rtsp_client_impl::timeout_set(std::chrono::milliseconds timeout)
{
    verify_access();

    _timeout = timeout;

    _timeout_timer.timeout_set(_timeout);
}

std::chrono::milliseconds _rtsp_client_impl::timeout()
{
    verify_access();

    return _timeout;
}

stream_statistics _rtsp_client_impl::stream_statistics_get(int stream_id) const
{
    verify_access();

    if(!_streaming_session)
    {
        throw std::invalid_argument("streaming session not configured");
    }

    return _streaming_session->stream_statistics_get(stream_id);
}

transport_protocol _rtsp_client_impl::protocol() const
{
    verify_access();

    return _protocol;
}

task<void> _rtsp_client_impl::play(std::vector<media_subsession> subsessions_)
{
    verify_access();

    auto subsession_ptr = std::make_shared<std::vector<media_subsession>>();

    *(subsession_ptr.get()) = std::move(subsessions_);

    return setup(subsession_ptr).then([=]
    {
        return create_for_task<size_t>(0, subsession_ptr->size(), [=](size_t i)
        {
            auto& subsession = subsession_ptr->at(i);

            _play_event = _current_event = task_completion_event<void>();

            _last_rtsp_command_id = _live_client->sendPlayCommand(*(subsession.__media_subsession)->live_media_subsession(), play_callback);

            _timeout_timer.start();

            return create_task(_play_event);
        });
    }).then([=]
    {
        _streaming_session = std::make_shared<_streaming_media_session_impl>(_session, std::move(*subsession_ptr.get()));

        _streaming_session->on_sample_set(_read_sample_delegate);
    });
}

std::string _rtsp_client_impl::password() const
{
    verify_access();

    return _password;
}

std::string _rtsp_client_impl::username() const
{
    verify_access();

    return _username;
}

void _rtsp_client_impl::credentials_set(const std::string& user_name, const std::string& pass)
{
    verify_access();

    _username = user_name;

    _password = pass;

    _authenticator.setUsernameAndPassword(_username.c_str(), _password.c_str());
}

void _rtsp_client_impl::on_sample_callback_set(read_sample_delegate callback)
{
    verify_access();
 
    _read_sample_delegate = std::move(callback);

    if(_streaming_session)
    {
        _streaming_session->on_sample_set(_read_sample_delegate);
    }
}

void _rtsp_client_impl::on_stream_closed_set(stream_closed_delegate callback)
{
    _stream_closed_delegate = std::move(callback);
    
    if (_streaming_session)
    {
        _streaming_session->on_stream_closed_set(_stream_closed_delegate);
    }
}

void _rtsp_client_impl::read_stream_sample()
{
    verify_access();

    if (!_streaming_session)
    {
        throw std::exception();
    }

    _streaming_session->read_stream_sample();
}



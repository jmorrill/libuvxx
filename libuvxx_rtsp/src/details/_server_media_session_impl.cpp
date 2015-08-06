#include "BasicUsageEnvironment.hh"

#include "details/_live_common.hpp"
#include "details/_live_server_media_session.hpp"
#include "details/_server_media_session_impl.hpp"
#include "details/_uvxx_task_scheduler.hpp"
#include "details/_h264_media_subsession.hpp"
#include "media_sample.hpp"
#include "details/_live_common.hpp"

using namespace uvxx::rtsp::details;

class _framed_source : public FramedSource
{
public:
    _framed_source() : FramedSource(*_get_live_environment().get())
    {
        FramedSource::afterGetting(this);
    }

public:

    virtual void doGetNextFrame() 
    {
        printf("do get next\n");
    }

    virtual void newFrameHandler1()
    {
        
    }
};

_server_media_session_impl::_server_media_session_impl()
{
    __live_server_media_session = _live_server_media_session_ptr(new _live_server_media_session(),
    [](_live_server_media_session* session)
    {
        /* since the live555 server manages the lifetime of this once
           it gets its grubby paws on the pointer */
        if(!session->is_externally_owned())
        {
            Medium::close(session);
        }
    });

    __live_server_media_session->on_session_closed(std::bind(&_server_media_session_impl::on_session_closed, this));
}

_server_media_session_impl::~_server_media_session_impl()
{
    if(__live_server_media_session)
    {
        __live_server_media_session->on_session_closed(nullptr);
    }
}

void _server_media_session_impl::on_session_closed()
{
    __live_server_media_session.reset(static_cast<_live_server_media_session*>(nullptr));
}

_server_media_session_impl::_server_media_session_impl(_server_media_session_impl&& rhs)
{
    *this = std::move(rhs);
}

_server_media_session_impl& _server_media_session_impl::operator=(_server_media_session_impl&& rhs)
{
    __live_server_media_session = std::move(rhs.__live_server_media_session);

    _descriptor = std::move(rhs._descriptor);

    return *this;
}

void _server_media_session_impl::set_media_descriptor(const uvxx::rtsp::media_descriptor& descriptor)
{
    _descriptor = descriptor;

    configure_session();
}

void _server_media_session_impl::deliver_sample(int stream_id, const uvxx::rtsp::media_sample& sample)
{
}

void _server_media_session_impl::configure_session()
{
    auto& streams = _descriptor.get_streams();

    for(auto& stream : streams)
    {
        if(stream.codec_name() == "H264")
        {
            __live_server_media_session->addSubsession(new _h264_media_subsession(new _framed_source()));
        }
    }
}
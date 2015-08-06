#include "BasicUsageEnvironment.hh"

#include "media_sample.hpp"
#include "details/_live_common.hpp"
#include "details/_live_server_media_session.hpp"
#include "details/_server_media_session_impl.hpp"
#include "details/_uvxx_task_scheduler.hpp"
#include "details/_h264_media_subsession.hpp"
#include "details/_live_common.hpp"

using namespace uvxx::rtsp::details;

_server_media_session_impl::_server_media_session_impl() : _source(nullptr)
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

    OutPacketBuffer::maxSize = 450 * 1024;

    _source = new _live_framed_source(std::bind(&_server_media_session_impl::on_framed_source_closed, this));
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

void _server_media_session_impl::on_framed_source_closed()
{
    _source = nullptr;
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
    if(_source)
    {
        _source->deliver_sample(sample);
    }
}


void _server_media_session_impl::configure_session()
{
    auto& streams = _descriptor.get_streams();

    for(auto& stream : streams)
    {
        if(stream.codec_name() == "H264")
        {
            __live_server_media_session->addSubsession(new _h264_media_subsession(std::bind(&_server_media_session_impl::create_framed_source, this, std::placeholders::_1)));
        }
    }
}

FramedSource* _server_media_session_impl::create_framed_source(unsigned client_id)
{
    return _source;
}
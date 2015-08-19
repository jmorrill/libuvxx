#include "BasicUsageEnvironment.hh"

#include "media_sample.hpp"

#include "details/_live_server_media_session.hpp"
#include "details/_server_media_session_impl.hpp"
#include "details/_server_media_subsession_factory.hpp"

using namespace uvxx::rtsp::details;

static uint32_t get_output_max_buffer_size()
{
    static const uint32_t MAX_OUTPUT_BUFFER_SIZE = 500 * 1024;

    return MAX_OUTPUT_BUFFER_SIZE;
}

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

    OutPacketBuffer::maxSize = get_output_max_buffer_size();
}

_server_media_session_impl::~_server_media_session_impl()
{
    if(__live_server_media_session)
    {
        __live_server_media_session->on_session_closed(nullptr);
    }
    
    for(const auto& streams: _stream_sources)
    {
        for(const auto& clients : streams.second)
        {
            clients.second->on_closed_set(nullptr);
        }
    }
}

bool _server_media_session_impl::is_session_active()
{
    return !_stream_sources.empty();
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
    auto streams_iterator = _stream_sources.find(stream_id);

    if(streams_iterator == _stream_sources.end())
    {
        return;
    }

    for(auto& client : streams_iterator->second)
    {
        client.second->deliver_sample(sample);
    }
}

bool _server_media_session_impl::has_live_session()
{
    return __live_server_media_session != nullptr;
}

void _server_media_session_impl::configure_session()
{
    auto& streams = _descriptor.get_streams();

    auto frame_source_created_callback = std::bind(&_server_media_session_impl::on_framed_source_created,
                                                   this,
                                                   std::placeholders::_1,
                                                   std::placeholders::_2,
                                                   std::placeholders::_3);

    for(const auto& stream : streams)
    {
        auto subsession = _create_server_media_subsession(stream);

        if(subsession)
        {
            subsession->framed_source_created_set(frame_source_created_callback);

            __live_server_media_session->addSubsession(subsession);
        }
    }
}

void _server_media_session_impl::on_framed_source_closed(int stream_id, unsigned client_session_id)
{
    auto stream_iterator = _stream_sources.find(stream_id);

    if (stream_iterator == _stream_sources.end())
    {
        return;
    }
    
    auto& sessions = stream_iterator->second;

    auto session_iterator = sessions.find(client_session_id);

    if(session_iterator == sessions.end())
    {
        return;
    }

    sessions.erase(session_iterator);
}

void _server_media_session_impl::on_framed_source_created(int stream_id, unsigned client_session_id, const std::shared_ptr<_live_framed_source>& source)
{
    source->on_closed_set(std::bind(&_server_media_session_impl::on_framed_source_closed, this, std::placeholders::_1, std::placeholders::_2));
    
    _stream_sources[stream_id][client_session_id] = source;
}
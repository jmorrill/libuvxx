#include "details/_event_dispatcher_impl.hpp"
#include "net/details/_stream_socket_impl.hpp"
#include "net/stream_socket.hpp"

using namespace std;
using namespace uvxx::pplx;

namespace uvxx { namespace net
{
    stream_socket::stream_socket() : 
        __stream_socket_impl(std::make_shared<details::_stream_socket_impl>())
    {
    }

    stream_socket::~stream_socket()
    {
            
    }

    stream_socket::stream_socket(stream_socket&& rhs) : event_dispatcher_object(std::move(rhs))
    {

    }

    stream_socket& stream_socket::operator=(stream_socket&& rhs)
    {
        __stream_socket_impl = std::move(__stream_socket_impl);
        return static_cast<stream_socket&>(event_dispatcher_object::operator=(std::move(rhs))); 
    }

    stream_socket::operator bool() const
    {
        return __stream_socket_impl != nullptr;
    }

    task<void> stream_socket::write_async(std::string const& buffer) const
    {
        return __stream_socket_impl->write_async(buffer);
    }

    task<int> stream_socket::read_async(io::memory_buffer const& buffer, int position, int count) const
    { 
        return __stream_socket_impl->read_async(buffer, position, count);
    }

    task<void> stream_socket::connect_async(std::string const& hostname, int port) const
    {
        return __stream_socket_impl->connect_async(hostname, port);
    }

    task<void> stream_socket::shutdown_async() const
    {
        return __stream_socket_impl->shutdown_async();
    }

    void stream_socket::no_read_delay_set(bool enable)
    {
        __stream_socket_impl->no_read_delay_set(enable);
    }

    bool stream_socket::no_read_delay_get()
    {
       return  __stream_socket_impl->no_read_delay_get();
    }

}
}
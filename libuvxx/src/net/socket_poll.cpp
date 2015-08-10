#include "details/_event_dispatcher_impl.hpp"
#include "net/details/_socket_poll_impl.hpp"
#include "net/socket_poll.hpp"

using namespace std;
using namespace uvxx::pplx;

namespace uvxx { namespace net
{
    socket_poll::socket_poll()
    {
            
    }

    socket_poll::socket_poll(int socket) : 
        __socket_poll(std::make_shared<details::_socket_poll_impl>(socket))
    {
    }

    socket_poll::socket_poll(socket_poll&& rhs)
    {
        *this = std::move(rhs);
    }

    socket_poll& socket_poll::operator=(socket_poll&& rhs)
    {
        __socket_poll = std::move(rhs.__socket_poll);
        return *this;
    }

    void socket_poll::start(socket_poll_event events)
    {
        __socket_poll->start(static_cast<int>(events));
    }

    void socket_poll::stop()
    {
        __socket_poll->stop();
    }

    socket_poll::operator bool()
    {
        return __socket_poll != nullptr;
    }

    bool socket_poll::operator==(std::nullptr_t /*rhs*/)
    {
        return operator bool();
    }

    void socket_poll::set_callback(std::function<void(int status, socket_poll_event events)> callback)
    {
        __socket_poll->set_callback(callback);
    }
}}
#include "details/_event_dispatcher_impl.hpp"
#include "details/_event_dispatcher_object_impl.hpp"
#include "net/details/_socket_poll_impl.hpp"
#include "net/socket_poll.hpp"

using namespace std;
using namespace uvxx::pplx;

namespace uvxx { namespace net
{
    socket_poll::socket_poll(int socket) : 
        __socket_poll(std::make_shared<details::_socket_poll_impl>(socket))
    {
    }

    socket_poll::socket_poll(socket_poll&& rhs) : event_dispatcher_object(std::move(rhs)), 
                                                  __socket_poll(std::move(rhs.__socket_poll))
    {

    }

    socket_poll& socket_poll::operator=(socket_poll&& rhs)
    {
        __socket_poll = std::move(__socket_poll);
        return static_cast<socket_poll&>(event_dispatcher_object::operator=(std::move(rhs))); 
    }

    void socket_poll::start(socket_poll_event events)
    {
        __socket_poll->start(static_cast<int>(events));
    }

    void socket_poll::stop()
    {
        __socket_poll->stop();
    }

    void socket_poll::set_callback(std::function<void(int status, int events)> callback)
    {
        __socket_poll->set_callback(callback);
    }
}}
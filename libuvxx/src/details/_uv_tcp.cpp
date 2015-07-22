#include "uvxx_exception.hpp"
#include "details/_uv_net_functions.hpp"
#include "details/_uv_tcp.hpp"
#include "details/_utilities.hpp"

namespace uvxx { namespace details
{
    _uv_tcp::_uv_tcp(_uv_loop* loop) : 
        _uv_stream(loop), 
        _connect_callback(connect_callback_delegate_t::create())
    {
        _loop = loop;

        uv_tcp_init(*loop, *this);
      
        _handle->data = this;
    }
    
    _uv_tcp::~_uv_tcp()
    {
        this->close();
    }
    
    void _uv_tcp::set_connect_callback(connect_callback_t connect_callback)
    {
        _connect_callback->callback_set(std::move(connect_callback));
    }

    int _uv_tcp::connect(const std::string& ip, int port)
    {
        ip4_addr addr = to_ip4_addr(ip, port);

        auto req = new uv_connect_t;

        req->data = _connect_callback.get();

        auto result = uv_tcp_connect(req, *this, reinterpret_cast<const sockaddr*>(&addr), 
        [](uv_connect_t* req, int status)
        {
            auto connect_callback = reinterpret_cast<connect_callback_delegate_t*>(req->data);

            SCOPE_EXIT(delete req);

            connect_callback->execute(status);
        });

        if (result == 0)
        {
            _connect_callback->busy_set(true);
        }
        else
        {
            delete req;
        }

        return result;
    }

    int _uv_tcp::connect6(const std::string& ip, int port)
    {
        ip6_addr addr = to_ip6_addr(ip, port);

        auto req = new uv_connect_t;

        req->data = this;

        auto result = uv_tcp_connect(req, *this, reinterpret_cast<const sockaddr*>(&addr), 
        [](uv_connect_t* req, int status)
        {
            auto this_tcp = reinterpret_cast<_uv_tcp*>(req->data);

            SCOPE_EXIT(delete req);

            this_tcp->_connect_callback->execute(status);
        });

        if (result == 0)
        {
            _connect_callback->busy_set(true);
        }
        else
        {
            delete req;
        }

        return result;
    }

    bool _uv_tcp::nodelay(bool enable)
    {
        return uv_tcp_nodelay(*this, enable ? 1 : 0) == 0;
    }

    bool _uv_tcp::keepalive(bool enable, unsigned int delay)
    {
        return uv_tcp_keepalive(*this, enable ? 1 : 0, delay) == 0;
    }

    bool _uv_tcp::simultanious_accepts(bool enable)
    {
        return uv_tcp_simultaneous_accepts(*this, enable ? 1 : 0) == 0;
    }
}}
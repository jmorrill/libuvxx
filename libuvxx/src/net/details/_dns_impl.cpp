#include "details/_event_dispatcher_impl.hpp"
#include "details/_event_dispatcher_object_impl.hpp"
#include "net/details/_dns_impl.hpp"
#include "details/_utilities.hpp"
#include "uvxx_exception.hpp"

using namespace uvxx;
using namespace uvxx::pplx;
using namespace uvxx::details;
using namespace uvxx::net::details;

namespace uvxx { namespace net { namespace details
{
    _dns_impl::_dns_impl() : _resolvehost_callback(resolvehost_callback_delegate_t::create())
    {
        _resolvehost_callback->callback_set([this](std::string const& ip_address, int error_code)
        {
            if (error_code == 0)
            {
                _resolvehost_completion_event.set(ip_address);
            }
            else
            {
                throw_for_code(error_code, _resolvehost_completion_event);
            }
        });
    }

    _dns_impl::~_dns_impl()
    {
        
    }

    void _dns_impl::on_getaddrinfo_callback(uv_getaddrinfo_t* req, int status, struct addrinfo* res)
    {
        auto callback = static_cast<resolvehost_callback_delegate_t*>(req->data);

        SCOPE_EXIT(delete req; uv_freeaddrinfo(res););

        if (status < 0)
        {
            callback->execute("", status);
            return;
        }

        char addr[17] = { '\0' };

        uv_ip4_name(reinterpret_cast<struct sockaddr_in*>(res->ai_addr), addr, 16);

       callback->execute(addr, status);
    }

    bool _dns_impl::begin_getaddrinfo(std::string const& host, std::string const& service)
    {
        struct addrinfo hints;
        hints.ai_family = PF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_flags = 0;

        uv_getaddrinfo_t* resolver = new uv_getaddrinfo_t;
        
        uv_loop_t* ploop = dispatcher()->_loop;
           
        resolver->data = _resolvehost_callback.get();

        int result = uv_getaddrinfo(ploop, 
                                    resolver, 
                                    on_getaddrinfo_callback, 
                                    host.c_str(), 
                                    service.c_str(), 
                                    &hints);

        if (result)
        {
            throw_for_code(result);
        }

        _resolvehost_callback->busy_set(true);

        return true;
    }

    task<std::string> _dns_impl::resolve_host_async(std::string const& host_name)
    {
        verify_access();

        if (_resolvehost_callback->busy_get())
        {
            throw uvxx_exception_with_code(UV_EBUSY);
        }

        begin_getaddrinfo(host_name);
        
        _resolvehost_completion_event.reset();

        return task<std::string>(_resolvehost_completion_event, 
                                 task_continuation_context::use_current());
    }

}}}
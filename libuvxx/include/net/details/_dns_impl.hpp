#pragma once
#include "details/_event_dispatcher_object_impl.hpp"
#include "pplx/pplxtasks.h"
#include "details/_uvxx_loop_callback.hpp"

namespace uvxx { namespace net { namespace details
{
    class _dns_impl : public uvxx::details::_event_dispatcher_object_impl
    {
         using resolvehost_callback_t            = std::function<void(std::string const &,int)>;
         using resolvehost_callback_delegate_t   = uvxx::details::_uvxx_loop_callback<resolvehost_callback_t>;
         using resolvehost_callback_delegate_ptr = uvxx::details::callback_unique_ptr<resolvehost_callback_delegate_t>;

    private:
         resolvehost_callback_delegate_ptr _resolvehost_callback;

    public:
        _dns_impl();

        ~_dns_impl();

        _dns_impl(const _dns_impl&) = delete;

        _dns_impl& operator=(const _dns_impl&) = delete;

        pplx::task<std::string> resolve_host_async(std::string const& host_name);

    private:
        pplx::task_completion_event<std::string> _resolvehost_completion_event;

        static void on_getaddrinfo_callback(uv_getaddrinfo_t* req, int status, struct addrinfo* res);

        bool begin_getaddrinfo(std::string const& host, std::string const& service = "");
    };
}}}
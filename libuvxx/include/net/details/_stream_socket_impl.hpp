#pragma once
#include <string>
#include "details/_event_dispatcher_timer_impl.hpp"
#include "details/_event_dispatcher_object_impl.hpp"
#include "details/_uv_tcp.hpp"
#include "../dns.hpp"
#include "pplx/pplxtasks.h"
#include "uvxx_exception.hpp"
#include "io/memory_buffer.hpp"

namespace uvxx { namespace details
{ 
    class _event_dispatcher_object_impl;
}}

namespace uvxx { namespace net { namespace details
{
    class _stream_socket_impl : public uvxx::details::_event_dispatcher_object_impl
    {
        void connect_callback(int exception_code);

        void read_callback(int len, int exception_code);

        void write_callback(int exception_code);

        void shutdown_callback(int exception_code);

    public:
        _stream_socket_impl();

        virtual ~_stream_socket_impl();

        uvxx::pplx::task<void> connect_async(std::string const& hostname, int port);

        uvxx::pplx::task<void> write_async(std::string const& buffer);

        uvxx::pplx::task<int> read_async(io::memory_buffer const& buffer, int position, int count);

        uvxx::pplx::task<void> shutdown_async();

        void no_read_delay_set(bool enable);

        bool no_read_delay_get();

    private:
        bool _no_read_delay;

        uvxx::details::_uv_tcp _tcp;

        uvxx::pplx::task_completion_event<void> _connect_event;

        uvxx::pplx::task_completion_event<int> _read_event;

        uvxx::pplx::task_completion_event<void> _write_event;

        uvxx::pplx::task_completion_event<void> _shutdown_event;

        uvxx::pplx::task_continuation_context _task_context;

        bool _busy_connecting;
    };
}}}
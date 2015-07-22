#pragma once
#include "event_dispatcher_object.hpp"
#include "pplx/pplxtasks.h"
#include "io/memory_buffer.hpp"

namespace uvxx { namespace net { namespace details
{
    class _stream_socket_impl;
}}}

namespace uvxx { namespace net
{
    class stream_socket : public event_dispatcher_object
    {
    public:
        stream_socket();

        virtual ~stream_socket();

        stream_socket(const stream_socket&) = default;

        stream_socket& operator=(const stream_socket&) = default;

        stream_socket(stream_socket&& rhs);

        stream_socket& operator=(stream_socket&& rhs);

        operator bool() const;

        uvxx::pplx::task<void> connect_async(std::string const& hostname, int port) const;

        uvxx::pplx::task<void> write_async(std::string const& buffer) const;

        uvxx::pplx::task<int> read_async(io::memory_buffer const& buffer, int position, int count) const;

        uvxx::pplx::task<void> shutdown_async() const;

        void no_read_delay_set(bool enable);

        bool no_read_delay_get();

    private:
       std::shared_ptr<details::_stream_socket_impl> __stream_socket_impl;
    };
}}
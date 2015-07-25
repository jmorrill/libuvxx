#include "details/_event_dispatcher_impl.hpp"
#include "details/_event_dispatcher_object_impl.hpp"
#include "net/details/_stream_socket_impl.hpp"
#include "io/memory_buffer.hpp"
#include "net/dns.hpp"

using namespace std;
using namespace uvxx::pplx;

namespace uvxx { namespace net { namespace details
{
    _stream_socket_impl::_stream_socket_impl() : _no_read_delay(false),
                                                 _tcp(&dispatcher()->_loop),
                                                 _task_context(task_continuation_context::use_current()),
                                                 _busy_connecting(false)
    {
        auto read_callback    = std::bind(&_stream_socket_impl::read_callback,
                                          this,
                                          std::placeholders::_1,
                                          std::placeholders::_2);        
                           
        auto connect_callback = std::bind(&_stream_socket_impl::connect_callback,
                                          this,
                                          std::placeholders::_1);

        auto write_callback   = std::bind(&_stream_socket_impl::write_callback,
                                          this,
                                          std::placeholders::_1);

        auto shutdown_callback = std::bind(&_stream_socket_impl::shutdown_callback,
                                           this,
                                           std::placeholders::_1);

        _tcp.set_connect_callback(std::move(connect_callback));
        _tcp.set_read_callback(std::move(read_callback));
        _tcp.set_write_callback(std::move(write_callback));
        _tcp.set_shutdown_callback(std::move(shutdown_callback));
    }

    _stream_socket_impl::~_stream_socket_impl()
    {
       
    }

    void _stream_socket_impl::connect_callback(int exception_code)
    {
        _busy_connecting = false;

        if (exception_code)
        {
            throw_for_code(exception_code, _connect_event);
        }
        else
        {
            _connect_event.set();
        }
    }

    void _stream_socket_impl::read_callback(int len, int exception_code)
    {
        if (exception_code)
        {
            throw_for_code(exception_code, _read_event);
        }
        else
        {
            _read_event.set(len);
        }
    }

    void _stream_socket_impl::write_callback(int exception_code)
    {
        if(exception_code)
        {
            throw_for_code(exception_code, _write_event);
        }
        else
        {
            _write_event.set();
        }
    }
    
    void _stream_socket_impl::shutdown_callback(int exception_code)
    {
        if(exception_code)
        {
            uvxx::throw_for_code(exception_code, _shutdown_event);
        }
        else
        {
            _shutdown_event.set();
        }
    }

    task<void> _stream_socket_impl::write_async(std::string const& buffer)
    {
        verify_access();
      
        int exception_code = _tcp.write(buffer);

        if (exception_code)
        {
            uvxx::throw_for_code(exception_code);
        }

        _write_event.reset();

        return create_task(_write_event, _task_context);
    }

    task<int> _stream_socket_impl::read_async(io::memory_buffer const& buffer, int position, int count)
    {
         verify_access();

         int exception_code = _tcp.read_start(buffer, buffer.length_get(), position, count, !_no_read_delay);

         if (exception_code)
         {
            uvxx::throw_for_code(exception_code);
         }

        _read_event.reset();

        return create_task(_read_event, _task_context);
    }

    task<void> _stream_socket_impl::connect_async(std::string const& hostname, int port)
    {
        verify_access();
        
        if (_busy_connecting)
        {
            throw uvxx::connection_already_in_progress();
        }

        _busy_connecting = true;

        _connect_event.reset();

        return dns::resolve_host_async(hostname).
        then([this, &hostname, port](task<std::string> resolved_host_ip_task)
        {
            std::string resolved_host_ip;

            try
            {
                resolved_host_ip = resolved_host_ip_task.get();
            }
            catch (uvxx::uv_exception_with_code const& e)
            {
                 _busy_connecting = false;

                 _connect_event.set_exception(e);

                 return;
            }
           
            int exception_code = _tcp.connect(resolved_host_ip, port);

            if (exception_code)
            {
                _busy_connecting = false;

                uvxx::throw_for_code(exception_code);
            }

        }, _task_context).then([this]
        {
            return create_task(_connect_event, _task_context);
        });
    }

    task<void> _stream_socket_impl::shutdown_async()
    {
        verify_access();

        int exception_code = _tcp.shutdown();

        if (exception_code)
        {
            uvxx::throw_for_code(exception_code);
        }

        _shutdown_event.reset();

        return create_task(_shutdown_event, _task_context);
    }

    void _stream_socket_impl::no_read_delay_set(bool enable)
    {
        _no_read_delay = enable;
    }

    bool _stream_socket_impl::no_read_delay_get()
    {
        return _no_read_delay;
    }

}}}
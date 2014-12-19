#pragma once
#include <exception>
#include <stdexcept>
#include <string>

namespace uvxx
{
    class uvxx_exception : public std::runtime_error
    {
    public:
        explicit uvxx_exception(const std::string & message) :
            std::runtime_error(message)
        {   }

        uvxx_exception() :
            std::runtime_error("")
        {   }
    };

    class uvxx_exception_with_code : public uvxx_exception
    {
    public:
        uvxx_exception_with_code(int code, const std::string& message) :
            uvxx_exception(message), _code(code)
        {   }

        explicit uvxx_exception_with_code(int code) :uvxx_exception(),
            _code(code)
        {   }

        explicit operator bool() const
        {
            return _code != 0;
        }

        int code()
        {
            return _code;
        }

    private:
        int _code;

    };

    class uv_exception_with_code : public uvxx_exception_with_code
    {
        int _code;
    public:
        uv_exception_with_code(int uv_code, const std::string& message) :
            uvxx_exception_with_code(uv_code, message), _code(uv_code)
        {   }

        explicit uv_exception_with_code(int uv_code);
    };

    void throw_for_code(int code);

    template<typename TaskCompletionEvent>
    inline void throw_for_code(int code, TaskCompletionEvent const& task_completion)
    {
        try
        {
            throw_for_code(code);
        }
        catch (...)
        {
            task_completion.set_exception(std::current_exception());
        }
    }

#define UV_EXCEPTION_WITH_CODE(name, code)             \
    class name : public uv_exception_with_code   \
        {                                            \
        public:                                  \
            name ();                             \
        };                                           

#define UV_EXCEPTION_WITH_CODE_IMPL(name, code)     \
    name::name() : uv_exception_with_code( code ) \
        {                                          \
                                                \
                                                  \
        }; 

    UV_EXCEPTION_WITH_CODE(argument_list_too_long, UV_E2BIG);
    UV_EXCEPTION_WITH_CODE(permission_denied, UV_EACCES);
    UV_EXCEPTION_WITH_CODE(address_in_use, UV_EADDRINUSE);
    UV_EXCEPTION_WITH_CODE(address_not_available, UV_EADDRNOTAVAIL);
    UV_EXCEPTION_WITH_CODE(address_family_not_supported, UV_EAFNOSUPPORT);
    UV_EXCEPTION_WITH_CODE(resource_temporarily_unavailable, UV_EAI_AGAIN);
    UV_EXCEPTION_WITH_CODE(bad_ai_flags, UV_EAI_BADFLAGS);
    UV_EXCEPTION_WITH_CODE(permanent_failure, UV_EAI_FAIL);
    UV_EXCEPTION_WITH_CODE(ai_family_not_supported, UV_EAI_FAMILY);
    UV_EXCEPTION_WITH_CODE(out_of_memory, UV_EAI_MEMORY);
    UV_EXCEPTION_WITH_CODE(no_address, UV_EAI_NODATA);
    UV_EXCEPTION_WITH_CODE(unknown_service, UV_EAI_NONAME);
    UV_EXCEPTION_WITH_CODE(argument_buffer_overflow, UV_EAI_OVERFLOW);
    UV_EXCEPTION_WITH_CODE(protocol_unknown, UV_EAI_PROTOCOL);
    UV_EXCEPTION_WITH_CODE(service_unavailable, UV_EAI_SERVICE);
    UV_EXCEPTION_WITH_CODE(socket_type_unsupported, UV_EAI_SOCKTYPE);
    UV_EXCEPTION_WITH_CODE(connection_already_in_progress, UV_EALREADY);
    UV_EXCEPTION_WITH_CODE(bad_file_descriptor, UV_EBADF);
    UV_EXCEPTION_WITH_CODE(resource_busy_or_locked, UV_EBUSY);
    UV_EXCEPTION_WITH_CODE(operation_canceled, UV_ECANCELED);
    UV_EXCEPTION_WITH_CODE(connection_aborted, UV_ECONNABORTED);
    UV_EXCEPTION_WITH_CODE(connection_refused, UV_ECONNREFUSED);
    UV_EXCEPTION_WITH_CODE(connection_reset, UV_ECONNRESET);
    UV_EXCEPTION_WITH_CODE(destination_address_required, UV_EDESTADDRREQ);
    UV_EXCEPTION_WITH_CODE(file_exists, UV_EEXIST);
    UV_EXCEPTION_WITH_CODE(bad_address, UV_EFAULT);
    UV_EXCEPTION_WITH_CODE(file_too_large, UV_EFBIG);
    UV_EXCEPTION_WITH_CODE(host_unreachable, UV_EHOSTUNREACH);
    UV_EXCEPTION_WITH_CODE(interrupted_system_call, UV_EINTR);
    UV_EXCEPTION_WITH_CODE(invalid_argument, UV_EINVAL);
    UV_EXCEPTION_WITH_CODE(io_error, UV_EIO);
    UV_EXCEPTION_WITH_CODE(socket_already_connected, UV_EISCONN);
    UV_EXCEPTION_WITH_CODE(illegal_directory_operation, UV_EISDIR);
    UV_EXCEPTION_WITH_CODE(too_many_symbolic_links, UV_ELOOP);
    UV_EXCEPTION_WITH_CODE(too_many_open_files, UV_EMFILE);
    UV_EXCEPTION_WITH_CODE(message_too_long, UV_EMSGSIZE);
    UV_EXCEPTION_WITH_CODE(name_too_long, UV_ENAMETOOLONG);
    UV_EXCEPTION_WITH_CODE(network_down, UV_ENETDOWN);
    UV_EXCEPTION_WITH_CODE(network_unreachable, UV_ENETUNREACH);
    UV_EXCEPTION_WITH_CODE(file_table_overflow, UV_ENFILE);
    UV_EXCEPTION_WITH_CODE(no_buffer_space_available, UV_ENOBUFS);
    UV_EXCEPTION_WITH_CODE(no_device, UV_ENODEV);
    UV_EXCEPTION_WITH_CODE(no_file_or_directory, UV_ENOENT);
    UV_EXCEPTION_WITH_CODE(not_enough_memory, UV_ENOMEM);
    UV_EXCEPTION_WITH_CODE(machine_not_on_network, UV_ENONET);
    UV_EXCEPTION_WITH_CODE(protocol_unavailable, UV_ENOPROTOOPT);
    UV_EXCEPTION_WITH_CODE(no_space_available, UV_ENOSPC);
    UV_EXCEPTION_WITH_CODE(function_not_implemented, UV_ENOSYS);
    UV_EXCEPTION_WITH_CODE(socket_not_connected, UV_ENOTCONN);
    UV_EXCEPTION_WITH_CODE(not_a_directory, UV_ENOTDIR);
    UV_EXCEPTION_WITH_CODE(directory_not_empty, UV_ENOTEMPTY);
    UV_EXCEPTION_WITH_CODE(not_socket, UV_ENOTSOCK);
    UV_EXCEPTION_WITH_CODE(socket_operation_not_supported, UV_ENOTSUP);
    UV_EXCEPTION_WITH_CODE(operation_not_permitted, UV_EPERM);
    UV_EXCEPTION_WITH_CODE(broken_pipe, UV_EPIPE);
    UV_EXCEPTION_WITH_CODE(protocol_error, UV_EPROTO);
    UV_EXCEPTION_WITH_CODE(protocol_not_supported, UV_EPROTONOSUPPORT);
    UV_EXCEPTION_WITH_CODE(protocol_wrong_for_socket, UV_EPROTOTYPE);
    UV_EXCEPTION_WITH_CODE(result_too_large, UV_ERANGE);
    UV_EXCEPTION_WITH_CODE(readonly_filesystem, UV_EROFS);
    UV_EXCEPTION_WITH_CODE(shutdown_error, UV_ESHUTDOWN);
    UV_EXCEPTION_WITH_CODE(invalid_seek, UV_ESPIPE);
    UV_EXCEPTION_WITH_CODE(no_such_process, UV_ESRCH);
    UV_EXCEPTION_WITH_CODE(connection_timed_out, UV_ETIMEDOUT);
    UV_EXCEPTION_WITH_CODE(text_file_busy, UV_ETXTBSY);
    UV_EXCEPTION_WITH_CODE(cross_device_link_not_permitted, UV_EXDEV);
    UV_EXCEPTION_WITH_CODE(unknown_error, UV_UNKNOWN);
    UV_EXCEPTION_WITH_CODE(end_of_file, UV_EOF);
    UV_EXCEPTION_WITH_CODE(no_such_device_or_address, UV_ENXIO);
    UV_EXCEPTION_WITH_CODE(too_many_links, UV_EMLINK);


}


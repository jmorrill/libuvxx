#include "uvxx_exception.hpp"
#include "details/_uvxx_internal.hpp"
using namespace uvxx;

uvxx::uv_exception_with_code::uv_exception_with_code(int uv_code) :
uvxx_exception_with_code(uv_code, std::string(uv_strerror(uv_code))), _code(uv_code)
{

}

#define THROW_FOR_CODE_CASE(EXCEPTION_TYPE, ERR_CODE)case ERR_CODE: throw EXCEPTION_TYPE();


void uvxx::throw_for_code(int code)
{
    switch (code)
    {
        THROW_FOR_CODE_CASE(argument_list_too_long, UV_E2BIG);
        THROW_FOR_CODE_CASE(permission_denied, UV_EACCES);
        THROW_FOR_CODE_CASE(address_in_use, UV_EADDRINUSE);
        THROW_FOR_CODE_CASE(address_not_available, UV_EADDRNOTAVAIL);
        THROW_FOR_CODE_CASE(address_family_not_supported, UV_EAFNOSUPPORT);
        THROW_FOR_CODE_CASE(resource_temporarily_unavailable, UV_EAI_AGAIN);
        THROW_FOR_CODE_CASE(bad_ai_flags, UV_EAI_BADFLAGS);
        THROW_FOR_CODE_CASE(permanent_failure, UV_EAI_FAIL);
        THROW_FOR_CODE_CASE(ai_family_not_supported, UV_EAI_FAMILY);
        THROW_FOR_CODE_CASE(out_of_memory, UV_EAI_MEMORY);
        THROW_FOR_CODE_CASE(no_address, UV_EAI_NODATA);
        THROW_FOR_CODE_CASE(unknown_service, UV_EAI_NONAME);
        THROW_FOR_CODE_CASE(argument_buffer_overflow, UV_EAI_OVERFLOW);
        THROW_FOR_CODE_CASE(protocol_unknown, UV_EAI_PROTOCOL);
        THROW_FOR_CODE_CASE(service_unavailable, UV_EAI_SERVICE);
        THROW_FOR_CODE_CASE(socket_type_unsupported, UV_EAI_SOCKTYPE);
        THROW_FOR_CODE_CASE(connection_already_in_progress, UV_EALREADY);
        THROW_FOR_CODE_CASE(bad_file_descriptor, UV_EBADF);
        THROW_FOR_CODE_CASE(resource_busy_or_locked, UV_EBUSY);
        THROW_FOR_CODE_CASE(operation_canceled, UV_ECANCELED);
        THROW_FOR_CODE_CASE(connection_aborted, UV_ECONNABORTED);
        THROW_FOR_CODE_CASE(connection_refused, UV_ECONNREFUSED);
        THROW_FOR_CODE_CASE(connection_reset, UV_ECONNRESET);
        THROW_FOR_CODE_CASE(destination_address_required, UV_EDESTADDRREQ);
        THROW_FOR_CODE_CASE(file_exists, UV_EEXIST);
        THROW_FOR_CODE_CASE(bad_address, UV_EFAULT);
        THROW_FOR_CODE_CASE(file_too_large, UV_EFBIG);
        THROW_FOR_CODE_CASE(host_unreachable, UV_EHOSTUNREACH);
        THROW_FOR_CODE_CASE(interrupted_system_call, UV_EINTR);
        THROW_FOR_CODE_CASE(invalid_argument, UV_EINVAL);
        THROW_FOR_CODE_CASE(io_error, UV_EIO);
        THROW_FOR_CODE_CASE(socket_already_connected, UV_EISCONN);
        THROW_FOR_CODE_CASE(illegal_directory_operation, UV_EISDIR);
        THROW_FOR_CODE_CASE(too_many_symbolic_links, UV_ELOOP);
        THROW_FOR_CODE_CASE(too_many_open_files, UV_EMFILE);
        THROW_FOR_CODE_CASE(message_too_long, UV_EMSGSIZE);
        THROW_FOR_CODE_CASE(name_too_long, UV_ENAMETOOLONG);
        THROW_FOR_CODE_CASE(network_down, UV_ENETDOWN);
        THROW_FOR_CODE_CASE(network_unreachable, UV_ENETUNREACH);
        THROW_FOR_CODE_CASE(file_table_overflow, UV_ENFILE);
        THROW_FOR_CODE_CASE(no_buffer_space_available, UV_ENOBUFS);
        THROW_FOR_CODE_CASE(no_device, UV_ENODEV);
        THROW_FOR_CODE_CASE(no_file_or_directory, UV_ENOENT);
        THROW_FOR_CODE_CASE(not_enough_memory, UV_ENOMEM);
        THROW_FOR_CODE_CASE(machine_not_on_network, UV_ENONET);
        THROW_FOR_CODE_CASE(protocol_unavailable, UV_ENOPROTOOPT);
        THROW_FOR_CODE_CASE(no_space_available, UV_ENOSPC);
        THROW_FOR_CODE_CASE(function_not_implemented, UV_ENOSYS);
        THROW_FOR_CODE_CASE(socket_not_connected, UV_ENOTCONN);
        THROW_FOR_CODE_CASE(not_a_directory, UV_ENOTDIR);
        THROW_FOR_CODE_CASE(directory_not_empty, UV_ENOTEMPTY);
        THROW_FOR_CODE_CASE(not_socket, UV_ENOTSOCK);
        THROW_FOR_CODE_CASE(socket_operation_not_supported, UV_ENOTSUP);
        THROW_FOR_CODE_CASE(operation_not_permitted, UV_EPERM);
        THROW_FOR_CODE_CASE(broken_pipe, UV_EPIPE);
        THROW_FOR_CODE_CASE(protocol_error, UV_EPROTO);
        THROW_FOR_CODE_CASE(protocol_not_supported, UV_EPROTONOSUPPORT);
        THROW_FOR_CODE_CASE(protocol_wrong_for_socket, UV_EPROTOTYPE);
        THROW_FOR_CODE_CASE(result_too_large, UV_ERANGE);
        THROW_FOR_CODE_CASE(readonly_filesystem, UV_EROFS);
        THROW_FOR_CODE_CASE(shutdown_error, UV_ESHUTDOWN);
        THROW_FOR_CODE_CASE(invalid_seek, UV_ESPIPE);
        THROW_FOR_CODE_CASE(no_such_process, UV_ESRCH);
        THROW_FOR_CODE_CASE(connection_timed_out, UV_ETIMEDOUT);
        THROW_FOR_CODE_CASE(text_file_busy, UV_ETXTBSY);
        THROW_FOR_CODE_CASE(cross_device_link_not_permitted, UV_EXDEV);
        THROW_FOR_CODE_CASE(unknown_error, UV_UNKNOWN);
        THROW_FOR_CODE_CASE(end_of_file, UV_EOF);
        THROW_FOR_CODE_CASE(no_such_device_or_address, UV_ENXIO);
        THROW_FOR_CODE_CASE(too_many_links, UV_EMLINK);
    default:
        throw unknown_error();
        break;
    }
}

UV_EXCEPTION_WITH_CODE_IMPL(argument_list_too_long, UV_E2BIG);
UV_EXCEPTION_WITH_CODE_IMPL(permission_denied, UV_EACCES);
UV_EXCEPTION_WITH_CODE_IMPL(address_in_use, UV_EADDRINUSE);
UV_EXCEPTION_WITH_CODE_IMPL(address_not_available, UV_EADDRNOTAVAIL);
UV_EXCEPTION_WITH_CODE_IMPL(address_family_not_supported, UV_EAFNOSUPPORT);
UV_EXCEPTION_WITH_CODE_IMPL(resource_temporarily_unavailable, UV_EAI_AGAIN);
UV_EXCEPTION_WITH_CODE_IMPL(bad_ai_flags, UV_EAI_BADFLAGS);
UV_EXCEPTION_WITH_CODE_IMPL(permanent_failure, UV_EAI_FAIL);
UV_EXCEPTION_WITH_CODE_IMPL(ai_family_not_supported, UV_EAI_FAMILY);
UV_EXCEPTION_WITH_CODE_IMPL(out_of_memory, UV_EAI_MEMORY);
UV_EXCEPTION_WITH_CODE_IMPL(no_address, UV_EAI_NODATA);
UV_EXCEPTION_WITH_CODE_IMPL(unknown_service, UV_EAI_NONAME);
UV_EXCEPTION_WITH_CODE_IMPL(argument_buffer_overflow, UV_EAI_OVERFLOW);
UV_EXCEPTION_WITH_CODE_IMPL(protocol_unknown, UV_EAI_PROTOCOL);
UV_EXCEPTION_WITH_CODE_IMPL(service_unavailable, UV_EAI_SERVICE);
UV_EXCEPTION_WITH_CODE_IMPL(socket_type_unsupported, UV_EAI_SOCKTYPE);
UV_EXCEPTION_WITH_CODE_IMPL(connection_already_in_progress, UV_EALREADY);
UV_EXCEPTION_WITH_CODE_IMPL(bad_file_descriptor, UV_EBADF);
UV_EXCEPTION_WITH_CODE_IMPL(resource_busy_or_locked, UV_EBUSY);
UV_EXCEPTION_WITH_CODE_IMPL(operation_canceled, UV_ECANCELED);
UV_EXCEPTION_WITH_CODE_IMPL(connection_aborted, UV_ECONNABORTED);
UV_EXCEPTION_WITH_CODE_IMPL(connection_refused, UV_ECONNREFUSED);
UV_EXCEPTION_WITH_CODE_IMPL(connection_reset, UV_ECONNRESET);
UV_EXCEPTION_WITH_CODE_IMPL(destination_address_required, UV_EDESTADDRREQ);
UV_EXCEPTION_WITH_CODE_IMPL(file_exists, UV_EEXIST);
UV_EXCEPTION_WITH_CODE_IMPL(bad_address, UV_EFAULT);
UV_EXCEPTION_WITH_CODE_IMPL(file_too_large, UV_EFBIG);
UV_EXCEPTION_WITH_CODE_IMPL(host_unreachable, UV_EHOSTUNREACH);
UV_EXCEPTION_WITH_CODE_IMPL(interrupted_system_call, UV_EINTR);
UV_EXCEPTION_WITH_CODE_IMPL(invalid_argument, UV_EINVAL);
UV_EXCEPTION_WITH_CODE_IMPL(io_error, UV_EIO);
UV_EXCEPTION_WITH_CODE_IMPL(socket_already_connected, UV_EISCONN);
UV_EXCEPTION_WITH_CODE_IMPL(illegal_directory_operation, UV_EISDIR);
UV_EXCEPTION_WITH_CODE_IMPL(too_many_symbolic_links, UV_ELOOP);
UV_EXCEPTION_WITH_CODE_IMPL(too_many_open_files, UV_EMFILE);
UV_EXCEPTION_WITH_CODE_IMPL(message_too_long, UV_EMSGSIZE);
UV_EXCEPTION_WITH_CODE_IMPL(name_too_long, UV_ENAMETOOLONG);
UV_EXCEPTION_WITH_CODE_IMPL(network_down, UV_ENETDOWN);
UV_EXCEPTION_WITH_CODE_IMPL(network_unreachable, UV_ENETUNREACH);
UV_EXCEPTION_WITH_CODE_IMPL(file_table_overflow, UV_ENFILE);
UV_EXCEPTION_WITH_CODE_IMPL(no_buffer_space_available, UV_ENOBUFS);
UV_EXCEPTION_WITH_CODE_IMPL(no_device, UV_ENODEV);
UV_EXCEPTION_WITH_CODE_IMPL(no_file_or_directory, UV_ENOENT);
UV_EXCEPTION_WITH_CODE_IMPL(not_enough_memory, UV_ENOMEM);
UV_EXCEPTION_WITH_CODE_IMPL(machine_not_on_network, UV_ENONET);
UV_EXCEPTION_WITH_CODE_IMPL(protocol_unavailable, UV_ENOPROTOOPT);
UV_EXCEPTION_WITH_CODE_IMPL(no_space_available, UV_ENOSPC);
UV_EXCEPTION_WITH_CODE_IMPL(function_not_implemented, UV_ENOSYS);
UV_EXCEPTION_WITH_CODE_IMPL(socket_not_connected, UV_ENOTCONN);
UV_EXCEPTION_WITH_CODE_IMPL(not_a_directory, UV_ENOTDIR);
UV_EXCEPTION_WITH_CODE_IMPL(directory_not_empty, UV_ENOTEMPTY);
UV_EXCEPTION_WITH_CODE_IMPL(not_socket, UV_ENOTSOCK);
UV_EXCEPTION_WITH_CODE_IMPL(socket_operation_not_supported, UV_ENOTSUP);
UV_EXCEPTION_WITH_CODE_IMPL(operation_not_permitted, UV_EPERM);
UV_EXCEPTION_WITH_CODE_IMPL(broken_pipe, UV_EPIPE);
UV_EXCEPTION_WITH_CODE_IMPL(protocol_error, UV_EPROTO);
UV_EXCEPTION_WITH_CODE_IMPL(protocol_not_supported, UV_EPROTONOSUPPORT);
UV_EXCEPTION_WITH_CODE_IMPL(protocol_wrong_for_socket, UV_EPROTOTYPE);
UV_EXCEPTION_WITH_CODE_IMPL(result_too_large, UV_ERANGE);
UV_EXCEPTION_WITH_CODE_IMPL(readonly_filesystem, UV_EROFS);
UV_EXCEPTION_WITH_CODE_IMPL(shutdown_error, UV_ESHUTDOWN);
UV_EXCEPTION_WITH_CODE_IMPL(invalid_seek, UV_ESPIPE);
UV_EXCEPTION_WITH_CODE_IMPL(no_such_process, UV_ESRCH);
UV_EXCEPTION_WITH_CODE_IMPL(connection_timed_out, UV_ETIMEDOUT);
UV_EXCEPTION_WITH_CODE_IMPL(text_file_busy, UV_ETXTBSY);
UV_EXCEPTION_WITH_CODE_IMPL(cross_device_link_not_permitted, UV_EXDEV);
UV_EXCEPTION_WITH_CODE_IMPL(unknown_error, UV_UNKNOWN);
UV_EXCEPTION_WITH_CODE_IMPL(end_of_file, UV_EOF);
UV_EXCEPTION_WITH_CODE_IMPL(no_such_device_or_address, UV_ENXIO);
UV_EXCEPTION_WITH_CODE_IMPL(too_many_links, UV_EMLINK);

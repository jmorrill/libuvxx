#pragma once
#include <string>
#include "details/_event_dispatcher_object_impl.hpp"
#include "details/_uv_file.hpp"
#include "pplx/pplxtasks.h"
#include "io/memory_buffer.hpp"
#include "fs/fs_info.hpp"

namespace uvxx { namespace fs { namespace details
{
    class _file_impl : public uvxx::details::_event_dispatcher_object_impl
    {
        void open_callback(int exception_code);

        void read_callback(const uint8_t* buf, size_t len, int exception_code);

        void write_callback(const uint8_t* buf, size_t len, int exception_code);

        void close_callback(int exception_code);

        void file_info_callback(file_info fileinfo, int exception_code);

    public:
        _file_impl();
        
        virtual ~_file_impl() = default;

        uvxx::pplx::task<void> open_async(std::string const& file_name, std::ios_base::openmode mode);

        uvxx::pplx::task<void> close_async();

        uvxx::pplx::task<size_t> read_async(uvxx::io::memory_buffer const& buffer, size_t start_pos, size_t count, int64_t file_position = -1);

        uvxx::pplx::task<size_t> write_async(uvxx::io::memory_buffer const& buffer, size_t start_pos, size_t count, int64_t file_position = -1);

        uvxx::pplx::task<size_t> write_async(const uint8_t* buffer, size_t buffer_size, size_t start_pos, size_t count, int64_t file_position = -1);

        int64_t file_position_get();

        void file_position_set(int64_t position);
        
        uvxx::pplx::task<void> delete_async(std::string const& file_name);

private:
        uvxx::pplx::task_completion_event<void> _open_event;

        uvxx::pplx::task_completion_event<void> _close_event;

        uvxx::pplx::task_completion_event<size_t> _read_event;

        uvxx::pplx::task_completion_event<size_t> _write_event;

        uvxx::pplx::task_continuation_context _task_context;

        std::string _file_name;

        uvxx::details::_uv_file _file;

        int64_t _file_position = 0;
    };
}}}
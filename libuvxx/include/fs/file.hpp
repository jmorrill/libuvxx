#pragma once
#include <string>
#include "event_dispatcher_object.hpp"
#include "pplx/pplxtasks.h"
#include "io/memory_buffer.hpp"
#include "fs_info.hpp"

namespace uvxx { namespace fs { namespace details
{
    class _file_impl;
}}}

namespace uvxx { namespace fs
{
    class file : public event_dispatcher_object
    {
    public:
        file();

        file(const file&) = default;

        file& operator=(const file&) = default;

        file(file&& rhs);

        file& operator=(file&& rhs);

        uvxx::pplx::task<void> open_async(std::string const& file_name, std::ios_base::openmode mode) const;

        uvxx::pplx::task<void> close_async() const;

        uvxx::pplx::task<int> read_async(io::memory_buffer const & buffer, int start_pos, int count) const;

        uvxx::pplx::task<int> write_async(io::memory_buffer const & buffer, int start_pos, int count) const;

        int64_t file_position_get() const;

        void file_position_set(size_t position) const;
        
        static uvxx::pplx::task<void> delete_async(std::string const& file_name);

        static uvxx::pplx::task<void> move_async(std::string const& source_file, std::string const& destination_file);

        static uvxx::pplx::task<uvxx::fs::file_info> get_file_info_async(std::string const& filename);

private:
        std::shared_ptr<uvxx::fs::details::_file_impl> __file_impl;
    };
}}
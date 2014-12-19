#pragma once
#include <string>
#include <vector>
#include "pplx/pplxtasks.h"

namespace uvxx { namespace fs
{
    using directory_entry_result = std::vector<std::string>;

    using directory_entry_result_ptr = std::shared_ptr<directory_entry_result>;

    class directory 
    {
        directory() = delete;

        ~directory() = delete;

    public:
        static uvxx::pplx::task<directory_entry_result_ptr> get_files_async(std::string const& path);

        static uvxx::pplx::task<directory_entry_result_ptr> get_files_async(std::string const& path, bool recursive);

        static uvxx::pplx::task<directory_entry_result_ptr> get_directories_async(std::string const& path);

        static uvxx::pplx::task<directory_entry_result_ptr> get_directories_async(std::string const& path, bool recursive);

        static uvxx::pplx::task<void> create_directory_async(std::string const& path);

        static uvxx::pplx::task<bool> exists_async(std::string const& path);

        static uvxx::pplx::task<void> delete_async(std::string const& path);

        static uvxx::pplx::task<void> delete_async(std::string const& path, bool recursive);
    };
}}
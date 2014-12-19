#include "fs/directory.hpp"
#include "fs/details/_directory_impl.hpp"

namespace uvxx { namespace fs 
{
    uvxx::pplx::task<directory_entry_result_ptr> directory::get_files_async(std::string const& path)
    {
        auto dir_impl = std::make_shared<uvxx::fs::details::_directory_impl>();

        return dir_impl->get_files_async(path).
        then([dir_impl](uvxx::pplx::task<directory_entry_result_ptr> result_task)
        {
            return result_task.get();
        });
    }

    uvxx::pplx::task<directory_entry_result_ptr> directory::get_files_async(std::string const& path, bool recursive)
    {
        auto dir_impl = std::make_shared<uvxx::fs::details::_directory_impl>();

        return dir_impl->get_files_async(path, recursive).
        then([dir_impl](uvxx::pplx::task<directory_entry_result_ptr> result_task)
        {
            return result_task.get();
        });
    }

    uvxx::pplx::task<directory_entry_result_ptr> directory::get_directories_async(std::string const& path)
    {
        auto dir_impl = std::make_shared<uvxx::fs::details::_directory_impl>();

        return dir_impl->get_directories_async(path).
        then([dir_impl](uvxx::pplx::task<directory_entry_result_ptr> result_task)
        {
            return result_task.get();
        });
    }

    uvxx::pplx::task<directory_entry_result_ptr> directory::get_directories_async(std::string const& path, bool recursive)
    {
        auto dir_impl = std::make_shared<uvxx::fs::details::_directory_impl>();

        return dir_impl->get_directories_async(path, recursive).
        then([dir_impl](uvxx::pplx::task<directory_entry_result_ptr> result_task)
        {
            return result_task.get();
        });
    }

    uvxx::pplx::task<void> directory::create_directory_async(std::string const& path)
    {
        auto dir_impl = std::make_shared<uvxx::fs::details::_directory_impl>();

        return dir_impl->create_directory_async(path).
        then([dir_impl](uvxx::pplx::task<void> result_task)
        {
            return result_task.get();
        });
    }

    uvxx::pplx::task<bool> directory::exists_async(std::string const& path)
    {
        auto dir_impl = std::make_shared<uvxx::fs::details::_directory_impl>();

        return dir_impl->exists_async(path).
        then([dir_impl](uvxx::pplx::task<bool> result_task)
        {
            return result_task.get();
        });
    }

    uvxx::pplx::task<void> directory::delete_async(std::string const& path)
    {
        auto dir_impl = std::make_shared<uvxx::fs::details::_directory_impl>();

        return dir_impl->delete_async(path).
        then([dir_impl](uvxx::pplx::task<void> result_task)
        {
            result_task.get();
        });
    }

    uvxx::pplx::task<void> directory::delete_async(std::string const& path, bool recursive)
    {
        auto dir_impl = std::make_shared<uvxx::fs::details::_directory_impl>();

        return dir_impl->delete_async(path, recursive).
        then([dir_impl](uvxx::pplx::task<void> result_task)
        {
            result_task.get();
        });
    }

}}
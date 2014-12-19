#pragma once
#include <string>
#include <vector>
#include <deque>
#include "details/_event_dispatcher_object_impl.hpp"
#include "pplx/pplxtasks.h"
#include "details/_uvxx_loop_callback.hpp"

namespace uvxx { namespace fs { namespace details
{
    enum class dir_entry_type
    {
        file = 1,
        directory = 2
    };

    struct scandir_entry
    {
        scandir_entry() = default;

        ~scandir_entry() = default;

        scandir_entry(const scandir_entry&) = default;

        scandir_entry& operator=(const scandir_entry&) = default;

        scandir_entry(scandir_entry&& rhs) : path(std::move(rhs.path))
        {
            entry_type = rhs.entry_type;
        }

        scandir_entry& operator=(scandir_entry&& rhs)
        {
            path = std::move(rhs.path);
            entry_type = rhs.entry_type;
            return *this;
        }

        dir_entry_type entry_type;

        std::string path;
    };

    using directory_entry_result = std::vector<std::string>;

    using directory_entry_result_ptr = std::shared_ptr<directory_entry_result>;

    using scandir_entry_result = std::vector<scandir_entry>;

    using scandir_entry_result_ptr = std::shared_ptr<scandir_entry_result>;

    class _directory_impl : public uvxx::details::_event_dispatcher_object_impl
    {
        struct entry_holder_t
        {
            std::shared_ptr<std::deque<std::string>> paths_to_search;

            directory_entry_result_ptr found_results;

            dir_entry_type search_type;

            bool recursive = false;
        };

        using scandir_callback_t            = std::function<void(std::shared_ptr<std::vector<scandir_entry>>&, int)>;
        using scandir_callback_delegate_t   = uvxx::details::_uvxx_loop_callback<scandir_callback_t>;
        using scandir_callback_delegate_ptr = uvxx::details::callback_unique_ptr<scandir_callback_delegate_t>;
        scandir_callback_delegate_ptr _scandir_callback;

        using mkdir_callback_t = std::function<void(std::string const&, int)>;
        using mkdir_callback_delegate_t = uvxx::details::_uvxx_loop_callback<mkdir_callback_t>;
        using mkdir_callback_delegate_ptr = uvxx::details::callback_unique_ptr<mkdir_callback_delegate_t>;
        mkdir_callback_delegate_ptr _mkdir_callback;

        using stat_callback_t = std::function<void(int)>;
        using stat_callback_delegate_t = uvxx::details::_uvxx_loop_callback<stat_callback_t>;
        using stat_callback_delegate_ptr = uvxx::details::callback_unique_ptr<stat_callback_delegate_t>;
        stat_callback_delegate_ptr _stat_callback;

        using rmdir_callback_t = std::function<void(int)>;
        using rmdir_callback_delegate_t = uvxx::details::_uvxx_loop_callback<rmdir_callback_t>;
        using rmdir_callback_delegate_ptr = uvxx::details::callback_unique_ptr<rmdir_callback_delegate_t>;
        rmdir_callback_delegate_ptr _rmdir_callback;

        uvxx::pplx::task_completion_event<scandir_entry_result_ptr> _get_entries_task_completion;

        uvxx::pplx::task_completion_event<void> _mkdir_task_completion;

        uvxx::pplx::task_completion_event<void> _stat_task_completion;

        uvxx::pplx::task_completion_event<void> _rmdir_task_completion;

        void on_scandir_callback(scandir_entry_result_ptr& entries, int status);

        void on_mkdir_callback(std::string const& path, int status);

        void on_stat_callback(int status);

        void on_rmdir_callback(int status);

    public:
        _directory_impl();

        virtual ~_directory_impl();

        uvxx::pplx::task<directory_entry_result_ptr> get_files_async(std::string const& path);

        uvxx::pplx::task<directory_entry_result_ptr> get_files_async(std::string const& path, bool recursive);

        uvxx::pplx::task<directory_entry_result_ptr> get_directories_async(std::string const& path);

        uvxx::pplx::task<directory_entry_result_ptr> get_directories_async(std::string const& path, bool recursive);

        uvxx::pplx::task<void> create_directory_async(std::string const& path);

        uvxx::pplx::task<bool> exists_async(std::string const& path);

        uvxx::pplx::task<void> delete_async(std::string const& path);

        uvxx::pplx::task<void> delete_async(std::string const& path, bool recursive);

    private:
        uvxx::pplx::task<directory_entry_result_ptr> get_entries_async(std::string const& path, dir_entry_type type, bool recursive);

        uvxx::pplx::task<scandir_entry_result_ptr> get_all_scandir_entries_async(std::string const& path);

        uvxx::pplx::task<scandir_entry_result_ptr> get_entries_async(uv_fs_t* req, std::string const& path);

        uvxx::pplx::task<void> get_stat_async(std::string const& path);

        uvxx::pplx::task<void> create_directory_internal_async(std::string const& path);

        static void on_scandir(uv_fs_t * req);

        static void on_mkdir(uv_fs_t * req);

        static void on_stat(uv_fs_t * req);

        static void on_rmdir(uv_fs_t * req);
    };
}}}
#include <deque>
#include <algorithm>
#include <iterator>
#include "fs/path.hpp"
#include "pplx/pplpp.hpp"
#include "fs/details/_directory_impl.hpp"
#include "details/_utilities.hpp"
#include "uvxx_exception.hpp"
#include "fs/details/_static_file_impl.hpp"

using namespace uvxx::pplx;

namespace uvxx { namespace fs { namespace details
{
    struct create_dir_data_holder
    {
        std::deque<std::string> stack_dir;

        std::deque<std::string> nonexisting_dirs;
    };

    _directory_impl::_directory_impl() : _scandir_callback (scandir_callback_delegate_t::create()),
                                         _mkdir_callback   (mkdir_callback_delegate_t::create()),
                                         _stat_callback    (stat_callback_delegate_t::create()),
                                         _rmdir_callback   (rmdir_callback_delegate_t::create())
    {
        _scandir_callback->callback_set([this](scandir_entry_result_ptr& result, int status){ on_scandir_callback(result, status); });
        
        _mkdir_callback->callback_set([this](std::string const& result, int status){ on_mkdir_callback(result, status); });
        
        _stat_callback->callback_set([this](int status){ on_stat_callback(status); });

        _rmdir_callback->callback_set([this](int status){ on_rmdir_callback(status); });
    }

    _directory_impl::~_directory_impl()
    {

    }

    void _directory_impl::on_scandir_callback(scandir_entry_result_ptr& entries, int status)
    {
        if (status == 0)
        {
            _get_entries_task_completion.set(entries);
        }
        else
        {
            throw_for_code(status, _get_entries_task_completion);
        }
    }

    void _directory_impl::on_mkdir_callback(std::string const& /*path*/, int status)
    {
        if (status == 0)
        {
            _mkdir_task_completion.set();
        }
        else
        {
            throw_for_code(status, _mkdir_task_completion);
        }
    }

    void _directory_impl::on_stat_callback(int status)
    {
        if (status == 0)
        {
            _stat_task_completion.set();
        }
        else
        {
            throw_for_code(status, _stat_task_completion);
        }
    }

    void _directory_impl::on_rmdir_callback(int status)
    {
         if (status == 0)
        {
            _rmdir_task_completion.set();
        }
        else
        {
            throw_for_code(status, _rmdir_task_completion);
        }
    }

    task<directory_entry_result_ptr> _directory_impl::get_files_async(std::string const& path)
    {
        return get_files_async(path, false);
    }

    task<directory_entry_result_ptr> _directory_impl::get_directories_async(std::string const& path)
    {
        return get_directories_async(path, false);
    }

    uvxx::pplx::task<directory_entry_result_ptr> _directory_impl::get_files_async(std::string const& path, bool recursive)
    {
        return get_entries_async(path, dir_entry_type::file, recursive);
    }

    uvxx::pplx::task<directory_entry_result_ptr> _directory_impl::get_directories_async(std::string const& path, bool recursive)
    {
        return get_entries_async(path, dir_entry_type::directory, recursive);
    }

    task<directory_entry_result_ptr> _directory_impl::get_entries_async(std::string const& path, dir_entry_type type, bool recursive)
    {
        auto entry_holder = std::make_shared<_directory_impl::entry_holder_t>();

        entry_holder->paths_to_search = std::make_shared<std::deque<std::string>>();

        entry_holder->found_results = std::make_shared<directory_entry_result>();

        entry_holder->recursive = recursive;

        entry_holder->search_type = type;

        return get_all_scandir_entries_async(path).
        then([entry_holder, this](task<scandir_entry_result_ptr> t)
        {
            auto found_entries = t.get();

            for (auto& item : *found_entries.get())
            {
                switch (item.entry_type)
                {
                case dir_entry_type::file:
                    if (entry_holder->search_type == dir_entry_type::file)
                    {
                        entry_holder->found_results->emplace_back(std::move(item.path));
                    }

                    break;

                case dir_entry_type::directory:
                    if (entry_holder->recursive)
                    {
                        entry_holder->paths_to_search->emplace_back(item.path);
                    }

                    if (entry_holder->search_type == dir_entry_type::directory)
                    {
                        entry_holder->found_results->emplace_back(std::move(item.path));
                    }

                    break;
                }
            }

            return create_iterative_task([entry_holder, this]()
            {
                if (entry_holder->paths_to_search->empty())
                {
                    throw_for_code(UV_EOF);
                }

                auto item = std::move(entry_holder->paths_to_search->front());

                entry_holder->paths_to_search->pop_front();

                return this->get_all_scandir_entries_async(item).
                then([entry_holder](task<scandir_entry_result_ptr> t)
                {
                    scandir_entry_result_ptr found_entries;

                    try
                    {
                        found_entries = t.get();
                    }
                    catch (...)
                    {
                        return;
                    }

                    for (auto& item : *found_entries.get())
                    {
                        switch (item.entry_type)
                        {
                        case dir_entry_type::file:
                            if (entry_holder->search_type == dir_entry_type::file)
                            {
                                entry_holder->found_results->emplace_back(std::move(item.path));
                            }
                            break;

                        case dir_entry_type::directory:
                            if (entry_holder->recursive)
                            {
                                entry_holder->paths_to_search->emplace_back(item.path);
                            }
                            if (entry_holder->search_type == dir_entry_type::directory)
                            {
                                entry_holder->found_results->emplace_back(std::move(item.path));
                            }
                            break;
                        }
                    }
                });
            });
        }).
        then([entry_holder](task<void> iterative_task) -> directory_entry_result_ptr
        {
            try
            {
                iterative_task.get();
            }
            catch (uvxx::end_of_file const&)
            {

            }

            return entry_holder->found_results;
        });
    }

    task<scandir_entry_result_ptr> _directory_impl::get_all_scandir_entries_async(std::string const& path)
    {
        auto req = new uv_fs_t;

        req->data = _scandir_callback.get();

        return get_entries_async(req, path);
    }

    task<scandir_entry_result_ptr> _directory_impl::get_entries_async(uv_fs_t* req, std::string const& path)
    {
        if (_scandir_callback->busy_get())
        {
            throw_for_code(UV_EBUSY);
        }

        uv_loop_t* ploop = dispatcher()->_loop;

        int result = uv_fs_scandir(ploop, req, path.c_str(), 0, on_scandir);

        if (result != 0)
        {
            delete req;

            throw_for_code(result);
        }

        _scandir_callback->busy_set(true);

        _get_entries_task_completion.reset();

        return task<scandir_entry_result_ptr>(_get_entries_task_completion);
    }

    uvxx::pplx::task<bool> _directory_impl::exists_async(std::string const& path)
    {
        return get_stat_async(path).then([](uvxx::pplx::task<void> t)
        {
            try
            {
                t.get();
            }
            catch (no_file_or_directory const&)
            {
            	return false;
            }
           
            return true;
        });
    }
    
    uvxx::pplx::task<void> _directory_impl::create_directory_async(std::string const& path)
    {
        int length = path.size();

        if (length >= 2 && path::is_directory_separator(path[length - 1]))
        {
            length--;
        }

        int lengthRoot = path::get_root_length(path);

#if _WIN32
        if (length == 2 && path::is_directory_separator(path[1]))
        {
            throw std::exception();
        }
#endif
        auto holder = std::make_shared<create_dir_data_holder>();

        if (length > lengthRoot)
        {
            int i = length - 1;

            while (i >= lengthRoot)
            {
                std::string dir = path.substr(0, i + 1);

                holder->stack_dir.emplace_back(std::move(dir));

                while (i > lengthRoot && path[i] != *path::DIRECTORY_SEPARATOR && path[i] != *path::ALT_DIRECTORY_SEPARATOR)
                {
                    i--;
                }

                i--;
            }
        }

        return create_iterative_task([holder, this]()
        {
            return this->exists_async(holder->stack_dir.back()).
            then([holder, this](task<bool> t)
            {

                bool exists = t.get();

                if (exists)
                {
                    holder->stack_dir.pop_back();
                }
                else
                {
                    holder->nonexisting_dirs.emplace_back(std::move(holder->stack_dir.back()));

                    holder->stack_dir.pop_back();
                }

                if (holder->stack_dir.empty())
                {
                    throw_for_code(UV_EOF);
                }
            });
        }).
        then([holder, this](task<void> iterative_task)
        {
            try
            {
                iterative_task.get();
            }
            catch (end_of_file const&)
            {
            	
            }

            if (holder->nonexisting_dirs.empty())
            {
                throw_for_code(UV_EOF);
            }

            return create_iterative_task([holder, this]
            {
                return create_directory_internal_async(holder->nonexisting_dirs.front()).
                then([holder](task<void> t)
                {
                    t.get();

                    holder->nonexisting_dirs.pop_front();

                    if (holder->nonexisting_dirs.empty())
                    {
                        throw_for_code(UV_EOF);
                    }
                });
            });
        }).
        then([](task<void> iterative_task)
        {
            try
            {
                iterative_task.get();
            }
            catch (end_of_file const&)
            {
            	
            }
        });
    }

    task<void> _directory_impl::create_directory_internal_async(std::string const& path)
    {
        if (_mkdir_callback->busy_get())
        {
            throw_for_code(UV_EBUSY);
        }

        uv_loop_t* ploop = dispatcher()->_loop;

        auto req = new uv_fs_t;

        req->data = _mkdir_callback.get();

        int result = uv_fs_mkdir(ploop, req, path.c_str(), 0, on_mkdir);

        if (result != 0)
        {
            delete req;

            throw_for_code(result);
        }

        _mkdir_callback->busy_set(true);

        _mkdir_task_completion.reset();

        return create_task(_mkdir_task_completion, task_continuation_context::use_current());
    }
    
    uvxx::pplx::task<void> _directory_impl::delete_async(std::string const& path, bool recursive)
    {
        if (!recursive)
        {
            return delete_async(path);
        }

        return get_files_async(path, true).
        then([](uvxx::pplx::task<directory_entry_result_ptr> result)
        {
            auto file_list = result.get();

            auto file_deque = std::make_shared<std::deque<std::string>>();

            std::move(file_list->begin(), file_list->end(), std::front_inserter(*file_deque));

            auto file_deleter = std::make_shared<_file_delete_async>();
           
            return uvxx::pplx::create_iterative_task([file_deleter, file_deque]()
            {
                if (file_deque->empty())
                {
                    throw_for_code(UV_EOF);
                }

                std::string file_name = std::move(file_deque->front());

                file_deque->pop_front();
                
                return file_deleter->delete_async(file_name);
            });
        }).
        then([path, this](task<void> iterative_task)
        {
            try
            {
                iterative_task.get();
            }
            catch (end_of_file)
            {
            	
            }
            catch (...)
            {
                throw;
            }

            return get_directories_async(path, true);
        }).
        then([this](task<directory_entry_result_ptr> result)
        {
            auto dir_list = result.get();

            auto dir_deque = std::make_shared<std::deque<std::string>>();

            std::move(dir_list->begin(), dir_list->end(), std::front_inserter(*dir_deque));

            return create_iterative_task([dir_deque, this]()
            {
                if (dir_deque->empty())
                {
                    throw_for_code(UV_EOF);
                }

                std::string dir_name = std::move(dir_deque->front());

                dir_deque->pop_front();

                return delete_async(dir_name);
            });
        }).then([this, path](task<void> t)
        {
            try
            {
                t.get();
            }
            catch (end_of_file const&)
            {
            	
            }
            catch (...)
            {
                throw;
            }
           

            return delete_async(path);
        });
    }
    
    uvxx::pplx::task<void> _directory_impl::delete_async(std::string const& path)
    {
        if (_stat_callback->busy_get())
        {
            throw_for_code(UV_EBUSY);
        }

        uv_loop_t* ploop = dispatcher()->_loop;

        auto req = new uv_fs_t;

        req->data = _rmdir_callback.get();

        int result = uv_fs_rmdir(ploop, req, path.c_str(), on_rmdir);

        if (result != 0)
        {
            delete req;

            throw_for_code(result);
        }

        _rmdir_callback->busy_set(true);

        _rmdir_task_completion.reset();

        return create_task(_rmdir_task_completion);
    }

    uvxx::pplx::task<void> _directory_impl::get_stat_async(std::string const& path)
    {
        if (_stat_callback->busy_get())
        {
            throw_for_code(UV_EBUSY);
        }

        uv_loop_t* ploop = dispatcher()->_loop;

        auto req = new uv_fs_t;

        req->data = _stat_callback.get();

        int result = uv_fs_stat(ploop, req, path.c_str(), on_stat);

        if (result != 0)
        {
            delete req;

            throw_for_code(result);
        }

        _stat_callback->busy_set(true);

        _stat_task_completion.reset();

        return create_task(_stat_task_completion);
    }
    
    void _directory_impl::on_scandir(uv_fs_t * req)
    {
        using namespace uvxx::details;

        int status = req->result;

        auto callback = static_cast<scandir_callback_delegate_t*>(req->data);

        SCOPE_EXIT(uv_fs_req_cleanup(req); delete req;);

        if (status < 0)
        {
            auto empty_result = scandir_entry_result_ptr();
            callback->execute(empty_result, status);
            return;
        }

        auto entries = std::make_shared<scandir_entry_result>();

        entries->reserve(status);

        uv_dirent_t dir;

        while (!uv_fs_scandir_next(req, &dir))
        {
            scandir_entry entry;
                
            entry.path = path::combine(req->path, dir.name);

            switch (dir.type)
            {
            case UV_DIRENT_DIR:
                entry.entry_type = dir_entry_type::directory;
                break;

            case UV_DIRENT_FILE:
                entry.entry_type = dir_entry_type::file;
                break;
            }
            entries->emplace_back(std::move(entry));
        }

        callback->execute(entries, 0);
    }
    
    void _directory_impl::on_mkdir(uv_fs_t * req)
    {
        using namespace uvxx::details;

        int status = req->result;

        auto callback = static_cast<mkdir_callback_delegate_t*>(req->data);

        SCOPE_EXIT(uv_fs_req_cleanup(req); delete req;);

        if (status < 0)
        {
            callback->execute(std::string(), status);
            return;
        }

        callback->execute(std::string(), 0);
    }

    void _directory_impl::on_stat(uv_fs_t * req)
    {
        using namespace uvxx::details;

        int status = req->result;

        auto callback = static_cast<stat_callback_delegate_t*>(req->data);

        SCOPE_EXIT(uv_fs_req_cleanup(req); delete req;);
        
        if (status < 0)
        {
            callback->execute(status);

            return;
        }

        callback->execute(0);
    }
        
    void _directory_impl::on_rmdir(uv_fs_t * req)
    {
        using namespace uvxx::details;

        int status = req->result;

        auto callback = static_cast<rmdir_callback_delegate_t*>(req->data);

        SCOPE_EXIT(uv_fs_req_cleanup(req); delete req;);

        if (status < 0)
        {
            callback->execute(status);

            return;
        }

        callback->execute(0);
    }
}}}
#pragma once
#include "_file_impl.hpp"
#include "fs/fs_info.hpp"

namespace uvxx { namespace fs { namespace details 
{ 
    class _file_delete_async : public uvxx::details::_event_dispatcher_object_impl
    {
        using unlink_callback_t            = std::function<void(int)>;
        using unlink_callback_delegate_t   = uvxx::details::_uvxx_loop_callback<unlink_callback_t>;
        using unlink_callback_delegate_ptr = uvxx::details::callback_unique_ptr<unlink_callback_delegate_t>;

    private:
        unlink_callback_delegate_ptr _unlink_callback;

        uvxx::pplx::task_completion_event<void> _unlink_task_completion;

        void unlink_callback(int exception_code);
       
    public:
        _file_delete_async();

        virtual ~_file_delete_async();

        uvxx::pplx::task<void> delete_async(std::string const& file_name);
    };

    class _file_move_async : public uvxx::details::_event_dispatcher_object_impl
    {
        using move_callback_t            = std::function<void(int)> ;
        using move_callback_delegate_t   = uvxx::details::_uvxx_loop_callback<move_callback_t>;
        using move_callback_delegate_ptr = uvxx::details::callback_unique_ptr<move_callback_delegate_t>;

    private:
        move_callback_delegate_ptr _move_callback;

        uvxx::pplx::task_completion_event<void> _move_task_completion;

        void move_callback(int exception_code);

    public:
        _file_move_async();

        virtual ~_file_move_async();

        uvxx::pplx::task<void> move_async(std::string const& source_name, std::string const& destination_name);
    };

    class _uv_file_stat : public uvxx::details::_event_dispatcher_object_impl
    {
        using stat_callback_t            = std::function<void(uv_fs_t*, int)>;
        using stat_callback_delegate_t   = uvxx::details::_uvxx_loop_callback<stat_callback_t>;
        using stat_callback_delegate_ptr = uvxx::details::callback_unique_ptr<stat_callback_delegate_t >;

        stat_callback_delegate_ptr _stat_callback_delegate;

         uvxx::pplx::task_completion_event<file_info> _stat_task_completion;

         void fs_stat_callback(uv_fs_t* req, int status)
         {
            uv_stat_t& stat = req->statbuf;

            uvxx::fs::file_info info(stat.st_size, 
                                     stat.st_birthtim.tv_sec, 
                                     stat.st_atim.tv_sec, 
                                     stat.st_mtim.tv_sec, 
                                     req->path);

            if (status)
            {
                _stat_task_completion.set(std::move(info));
            }
            else
            {
                throw_for_code(status, _stat_task_completion);
            }
         }

         void stat_get(std::string const& filename)
         {
             auto req = new uv_fs_t;

            req->data = _stat_callback_delegate.get();

            uv_loop_t* ploop = dispatcher()->_loop;

            int result = uv_fs_stat(ploop, req, filename.c_str(), [](uv_fs_t* req)
            {
                using namespace uvxx::details;

                int result = req->result;

                auto callback = static_cast<stat_callback_delegate_t*>(req->data);

                SCOPE_EXIT(uv_fs_req_cleanup(req); delete req);

                callback->execute(req, result);
            });

            if (result)
            {
                throw uv_exception_with_code(result);
            }

            _stat_callback_delegate->busy_set(true);
         }
    public:
         _uv_file_stat(): _stat_callback_delegate(stat_callback_delegate_t::create())
         {
            _stat_callback_delegate->callback_set([=](uv_fs_t* req, int status)
            {
                fs_stat_callback(req, status);
            });
        }

        uvxx::pplx::task<uvxx::fs::file_info> get_file_info_async(std::string const& filename)
        {
            verify_access();

            stat_get(filename);

            _stat_task_completion.reset();

            return create_task(_stat_task_completion, uvxx::pplx::task_continuation_context::use_current());
        }
    };

}}}
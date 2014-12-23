#pragma once
#include "_file_impl.hpp"
#include "fs/fs_info.hpp"

namespace uvxx { namespace fs { namespace details 
{ 
    class _file_delete_async : public uvxx::details::_event_dispatcher_object_impl
    {
        using unlink_callback_t = std::function<void(int)>;
        using unlink_callback_delegate_t = uvxx::details::_uvxx_loop_callback<unlink_callback_t>;
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

    class _uv_file_stat :  public uvxx::details::_event_dispatcher_object_impl
    {
    private:
        using stat_callback_t            = std::function<void(uv_stat_t*, int)>;
        using stat_callback_delegate_t   = uvxx::details::_uvxx_loop_callback<stat_callback_t>;
        using stat_callback_delegate_ptr = uvxx::details::callback_unique_ptr<stat_callback_delegate_t >;

        stat_callback_delegate_ptr _stat_callback_delegate;

        uvxx::pplx::task_completion_event<file_info> _stat_task_completion;

        void fs_stat_callback(uv_stat_t* stat, int status);

        void stat_get(std::string const& filename);

    public:
        _uv_file_stat();

        uvxx::pplx::task<file_info> get_file_info_async(std::string const& filename);
    };
}}}
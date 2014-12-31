#include "fs/details/_static_file_impl.hpp"

namespace uvxx { namespace fs { namespace details
{
    void _file_delete_async::unlink_callback(int exception_code)
    {
        if (exception_code == 0)
        {
            _unlink_task_completion.set();
        }
        else
        {
            throw_for_code(exception_code, _unlink_task_completion);
        }
    }

    _file_delete_async::_file_delete_async() : _unlink_callback(unlink_callback_delegate_t::create())
    {
        auto unlink_cb = std::bind(&_file_delete_async::unlink_callback,
            this,
            std::placeholders::_1);

        _unlink_callback->callback_set(unlink_cb);
    }

    _file_delete_async::~_file_delete_async()
    {
    }

    uvxx::pplx::task<void> _file_delete_async::delete_async(std::string const& file_name)
    {
        if (_unlink_callback->busy_get())
        {
            throw_for_code(UV_EBUSY);
        }

        uv_loop_t* ploop = dispatcher()->_loop;

        auto req = new uv_fs_t;

        req->data = _unlink_callback.get();

        int result = uv_fs_unlink(ploop, req, file_name.c_str(), [](uv_fs_t* req)
        {
            using namespace uvxx::details;

            int status = req->result;

            auto callback = static_cast<unlink_callback_delegate_t*>(req->data);

            SCOPE_EXIT(uv_fs_req_cleanup(req); delete req;);

            if (status < 0)
            {
                callback->execute(status);

                return;
            }

            callback->execute(0);
        });

        if (result != 0)
        {
            delete req;

            throw_for_code(result);
        }

        _unlink_callback->busy_set(true);

        _unlink_task_completion.reset();

        return create_task(_unlink_task_completion, uvxx::pplx::task_continuation_context::use_current());
    }

    void _file_move_async::move_callback(int exception_code)
    {
        if (exception_code == 0)
        {
            _move_task_completion.set();
        }
        else
        {
            throw_for_code(exception_code, _move_task_completion);
        }
    }

    _file_move_async::_file_move_async() : _move_callback(move_callback_delegate_t::create())
    {
        auto unlink_cb = std::bind(&_file_move_async::move_callback,
                                   this,
                                   std::placeholders::_1);

        _move_callback->callback_set(unlink_cb);
    }

    _file_move_async::~_file_move_async()
    {
    }

    uvxx::pplx::task<void> _file_move_async::move_async(std::string const& source_file, std::string const& destination_file)
    {
        if (_move_callback->busy_get())
        {
            throw_for_code(UV_EBUSY);
        }

        uv_loop_t* ploop = dispatcher()->_loop;

        auto req = new uv_fs_t;

        req->data = _move_callback.get();

        int result = uv_fs_rename(ploop, req, source_file.c_str(), destination_file.c_str(), [](uv_fs_t* req)
        {
            using namespace uvxx::details;

            int status = req->result;

            auto callback = static_cast<move_callback_delegate_t*>(req->data);

            SCOPE_EXIT(uv_fs_req_cleanup(req); delete req;);

            if (status < 0)
            {
                callback->execute(status);

                return;
            }

            callback->execute(0);
        });

        if (result != 0)
        {
            delete req;

            throw_for_code(result);
        }

        _move_callback->busy_set(true);

        _move_task_completion.reset();

        return create_task(_move_task_completion, uvxx::pplx::task_continuation_context::use_current());
    }

}}}
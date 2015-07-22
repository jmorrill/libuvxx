#include "details/_uv_file.hpp"

namespace uvxx { namespace details 
{
    _uv_file::_uv_file(_uv_loop* loop) :
        _open_callback_delegate(open_callback_delegate_t::create()),
        _close_callback_delegate(close_callback_delegate_t::create()),
        _read_callback_delegate(read_callback_delegate_t::create()),
        _write_callback_delegate(write_callback_delegate_t::create())
    {
        _loop = loop;
        _handle->data = this;
    }

    _uv_file::~_uv_file()
    {

    }

    void _uv_file::open_callback_set(open_callback_t open_callback)
    {
        _open_callback_delegate->callback_set(std::move(open_callback));
    }

    void _uv_file::read_callback_set(read_callback_t read_callback)
    {
        _read_callback_delegate->callback_set(std::move(read_callback));
    }

    void _uv_file::close_callback_set(close_callback_t close_callback)
    {
        _close_callback_delegate->callback_set(std::move(close_callback));
    }
    
    void _uv_file::write_callback_set(write_callback_t write_callback)
    {
        _write_callback_delegate->callback_set(std::move(write_callback));
    }

    void _uv_file::open(std::string const& file_name, int flags, int mode)
    {
        //int m = 400 | 200;
        //int f = O_WRONLY | O_CREAT;

        int result = uv_fs_open(*_loop, this->_handle, file_name.c_str(), flags, mode,
        [](uv_fs_t * req)
        {

            auto this_file = static_cast<_uv_file*>(req->data);

            int result = req->result;

            this_file->_file = result;

            if (result == -1)
            {
                this_file->_open_callback_delegate->execute(result);
                return;
            }

            uv_fs_req_cleanup(req);
            this_file->_open_callback_delegate->execute(0);
        });

        if (result)
        {
            throw uv_exception_with_code(result);
        }

        _open_callback_delegate->busy_set(true);
    }

    void _uv_file::close()
    {
        int result = uv_fs_close(*_loop, _handle, _file,
            [](uv_fs_t* req)
        {
            uv_fs_req_cleanup(req);

            auto this_file = static_cast<_uv_file*>(req->data);

            int result = req->result;

            this_file->_close_callback_delegate->execute(0);
        });

        if (result)
        {
            throw uv_exception_with_code(result);
        }

        _close_callback_delegate->busy_set(true);
    }

    void _uv_file::read(uint8_t* buffer, size_t buffer_size, size_t start_pos, size_t count, int64_t file_position)
    {
        if (!buffer)
        {
            throw uvxx_exception("buffer cannot be null");
        }

        if (!count)
        {
            throw uvxx_exception("count cannot be less than or equal to 0");
        }

        if (count + start_pos > buffer_size)
        {
            throw uvxx_exception("count and position cannot be greater than the buffer size");
        }

        uv_buf_t bufs[1];

        bufs[0].base = reinterpret_cast<char*>(buffer + start_pos);

        bufs[0].len = count;
       
        int result = uv_fs_read(*_loop, _handle, _file, bufs, 1, file_position,
            [](uv_fs_t* req)
        {
            int result = req->result;
            auto this_file = static_cast<_uv_file*>(req->data);

            if (result < 0)
            {
                this_file->_read_callback_delegate->execute(static_cast<const uint8_t*>(nullptr), 0, result);
            }
            else
            {
                uv_fs_req_cleanup(req);
                throw std::exception();
                //this_file->_read_callback_delegate->execute(reinterpret_cast<const uint8_t*>(req->fs.info.bufs[0].base), req->result, 0);
            }
        });

        if (result)
        {
            throw uv_exception_with_code(result);
        }

        _read_buffer = buffer;

        _read_count = count;

        _file_position = start_pos;

        _read_callback_delegate->busy_set(true);
    }

    void _uv_file::write(const uint8_t* buffer, size_t buffer_size, size_t start_pos, size_t count, int64_t file_position)
    {
        if (!buffer)
        {
            throw uvxx_exception("buffer cannot be null");
        }

        if (!count)
        {
            throw uvxx_exception("count cannot be less than or equal to 0");
        }

        if (count + start_pos > buffer_size)
        {
            throw uvxx_exception("count and position cannot be greater than the buffer size");
        }

        uv_buf_t bufs[1];

        bufs[0].base = reinterpret_cast<char*>(const_cast<uint8_t*>(buffer) + start_pos);

        bufs[0].len = count;

        int result = uv_fs_write(*_loop, this->_handle, _file, bufs, 1, file_position, [](uv_fs_t* req)
        {
            int result = req->result;
            auto this_file = static_cast<_uv_file*>(req->data);

            if (result < 0)
            {
                this_file->_write_callback_delegate->execute(static_cast<const uint8_t*>(nullptr), 0, result);
            }
            else
            {
                uv_fs_req_cleanup(req);
                throw std::exception();
               // this_file->_write_callback_delegate->execute(reinterpret_cast<const uint8_t*>(req->fs.info.bufs[0].base), req->result, 0);
            }
        });

        if (result)
        {
            throw uv_exception_with_code(result);
        }

        _write_callback_delegate->busy_set(true);
    }
}}
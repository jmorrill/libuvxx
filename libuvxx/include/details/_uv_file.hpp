#pragma once
#include "_uvxx_internal.hpp"
#include "_uv_handle.hpp"
#include "_uv_loop.hpp"
#include "_uvxx_loop_callback.hpp"
#include "uvxx_exception.hpp"
#include "_utilities.hpp"

namespace uvxx { namespace details
{
    

    class _uv_file : public _uv_handle<uv_fs_t>
    {
    public:
        using open_callback_t             = std::function<void(int)>;
        using open_callback_delegate_t    = _uvxx_loop_callback<open_callback_t>;
        using open_callback_delegate_ptr  = callback_unique_ptr<open_callback_delegate_t>;

        using close_callback_t            = std::function<void(int)>;
        using close_callback_delegate_t   = _uvxx_loop_callback<close_callback_t>;
        using close_callback_delegate_ptr = callback_unique_ptr<close_callback_delegate_t>;

        using read_callback_t             = std::function<void(const uint8_t* buf, size_t len, int)>;
        using read_callback_delegate_t    = _uvxx_loop_callback<read_callback_t>;
        using read_callback_delegate_ptr  = callback_unique_ptr<read_callback_delegate_t>;

        using write_callback_t            = std::function<void(const uint8_t* buf, size_t len, int)>;
        using write_callback_delegate_t   = _uvxx_loop_callback<write_callback_t>;
        using write_callback_delegate_ptr = callback_unique_ptr<write_callback_delegate_t>;

    private:
        open_callback_delegate_ptr  _open_callback_delegate;

        close_callback_delegate_ptr _close_callback_delegate;

        read_callback_delegate_ptr  _read_callback_delegate;

        write_callback_delegate_ptr _write_callback_delegate;


    public:
        _uv_file(_uv_loop* l);

        ~_uv_file();

        void open_callback_set(open_callback_t open_callback);

        void read_callback_set(read_callback_t read_callback);

        void write_callback_set(write_callback_t write_callback);

        void close_callback_set(close_callback_t close_callback);

        void open(std::string const& file_name, int flags, int mode);

        void close();

        void read(uint8_t* buffer, size_t buffer_size, size_t start_pos, size_t count, int64_t file_position = -1);
      
        void write(const uint8_t* buffer, size_t buffer_size, size_t start_pos, size_t count, int64_t file_position = -1);
    private:
        uint8_t * _read_buffer;

        uint8_t* _read_buffer_size;

        size_t _file_position = 0;

        int _read_count = 0;

        uv_file _file;

        _uv_loop* _loop;
    };
}}
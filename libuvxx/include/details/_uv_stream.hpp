#pragma once
#include <iostream>
#include <vector>

#include "_uvxx_internal.hpp"
#include "_uv_handle.hpp"
#include "_uvxx_loop_callback.hpp"
#include "_uvxx_internal.hpp"
#include "../uvxx_exception.hpp"
#include "_utilities.hpp"
#include "_uv_timer.hpp"

namespace uvxx { namespace details
{
    template<typename HANDLE_T>
    class _uv_stream : public _uv_handle<HANDLE_T>
    {
        struct stream_read_info
        {
            stream_read_info() = default;
            stream_read_info(const stream_read_info &) = default;
            stream_read_info& operator=(const stream_read_info&) = default;

            uint8_t*  read_buffer;

            int read_buffer_size = 0;
            int read_buffer_offset = 0;
            int read_requested_bytes = 0;
            int bytes_read_this_request = 0;
            bool try_fill_read_buffer = false;

            int read_call_count = 0;
            int64_t total_bytes_read = 0ll;
        };

    public:
        using read_callback_t     = std::function<void(int len, int)>;
        using write_callback_t    = std::function<void(int)>;
        using shutdown_callback_t = std::function<void(int)>;

    private:
        using read_callback_delegate         = _extra_data_uvxx_loop_callback<read_callback_t, stream_read_info>;
        using read_callback_delegate_ptr     = callback_unique_ptr<read_callback_delegate>;
                                             
        using write_callback_delegate        = _uvxx_loop_callback<write_callback_t>;
        using write_callback_delegate_ptr    = callback_unique_ptr<write_callback_delegate>;
                                             
        using shutdown_callback_delegate     = _uvxx_loop_callback<shutdown_callback_t>;
        using shutdown_callback_delegate_ptr = callback_unique_ptr<shutdown_callback_delegate>;

        read_callback_delegate_ptr     _read_callback;
        write_callback_delegate_ptr    _write_callback;
        shutdown_callback_delegate_ptr _shutdown_callback;

        _uv_timer _read_timeout_timer;

        void read_timeout_timer_callback()
        {
            read_stop_if_read_any();
        }


    protected:
        _uv_stream(_uv_loop* loop) :
            _uv_handle<HANDLE_T>(), _write_callback(write_callback_delegate::create()),
                                    _shutdown_callback(shutdown_callback_delegate::create()),
                                    _read_callback(read_callback_delegate::create()),
                                    _read_timeout_timer(loop, std::bind(&_uv_stream::read_timeout_timer_callback, this))
        {
           
            std::unique_ptr<stream_read_info> read_info(new stream_read_info);

            _read_callback->extra_data_set(std::move(read_info));
        }

        virtual ~_uv_stream()
        {
            _read_timeout_timer.stop();
            this->close();
        }
    public:

        void set_read_callback(read_callback_t read_callback)
        {
            auto func = std::bind([this](int len, int status, read_callback_t read_callback)
            {
                _read_timeout_timer.stop();
                read_callback(len, status);
            }, std::placeholders::_1, std::placeholders::_2, std::move(read_callback));

           _read_callback->callback_set(func);
        }

        void set_write_callback(write_callback_t write_callback)
        {
            _write_callback->callback_set(std::move(write_callback));
        }

        void set_shutdown_callback(shutdown_callback_t shutdown_callback)
        {
            _shutdown_callback->callback_set(std::move(shutdown_callback));
        }

        int read_start(uint8_t * buffer, int buffer_size, int position, int count, bool fill_buffer)
        {
            if (!buffer)
            {
                throw uvxx_exception("buffer cannot be null");
            }

            if (position < 0)
            {
                throw uvxx_exception("position cannot be less than or equal to 0");
            }

            if (count < 0 || !count)
            {
                throw uvxx_exception("count cannot be less than or equal to 0");
            }

            if (count + position > buffer_size)
            {
                throw uvxx_exception("count and position cannot be greater than the buffer size");
            }

            auto read_stream_info = _read_callback->extra_data_get();

            read_stream_info->try_fill_read_buffer    = fill_buffer;
            read_stream_info->read_buffer             = buffer;
            read_stream_info->read_requested_bytes    = count;
            read_stream_info->read_buffer_offset      = position;
            read_stream_info->read_buffer_size        = buffer_size;
            read_stream_info->bytes_read_this_request = 0;

            this->_handle->data = _read_callback.get();

            auto result = uv_read_start(reinterpret_cast<uv_stream_t*>(this->_handle),
            [](uv_handle_t* h, size_t suggested_size, uv_buf_t* buf)
            {
                auto callback = reinterpret_cast<read_callback_delegate*>(h->data);

                if (!callback || callback->abort_get())
                {
                    buf->len = 0;

                    buf->base = nullptr;

                    return;
                }

                stream_read_info* read_info = callback->extra_data_get();

                auto read_count    = read_info->read_requested_bytes;
                auto read_position = read_info->read_buffer_offset;
                auto read_buffer   = read_info->read_buffer;
                auto bytes_read    = read_info->bytes_read_this_request;

                buf->base = reinterpret_cast<char*>(read_buffer + read_position);

                buf->len = read_count - bytes_read;
            },
            [](uv_stream_t* h, ssize_t nread, const uv_buf_t* buf) 
            {
                auto callback = reinterpret_cast<read_callback_delegate*>(h->data);

                if (!callback)
                {
                    uv_read_stop(reinterpret_cast<uv_stream_t*>(h));
                    return;
                }

                stream_read_info* read_info = callback->extra_data_get();

                read_info->total_bytes_read += nread;
                read_info->read_call_count++;

                if (read_info->read_call_count % 80000 == 0)
                {
                    int64_t avg = read_info->total_bytes_read/read_info->read_call_count;
                    printf("avg pkt size: %llu\n", avg);
                    read_info->read_call_count  = 0;
                    read_info->total_bytes_read = 0;
                }
               
                if (nread < 0 || callback->abort_get())
                {
                    uv_read_stop(reinterpret_cast<uv_stream_t*>(h));

                    if (nread < 0 && read_info->bytes_read_this_request > 0)
                    {
                        callback->execute(read_info->bytes_read_this_request, 0);
                    }
                    else
                    {
                        callback->execute(0, nread);
                    }
                }
                else if (nread >= 0)
                {
                    read_info->read_buffer_offset      += nread;
                    read_info->bytes_read_this_request += nread;

                    if(read_info->bytes_read_this_request - read_info->read_requested_bytes == 0 || 
                      !read_info->try_fill_read_buffer)
                    {
                        uv_read_stop(reinterpret_cast<uv_stream_t*>(h));

                        callback->execute(read_info->bytes_read_this_request, 0);

                        read_info->bytes_read_this_request = 0;
                    }
                }
            });

            if (result == 0)
            {
                _read_callback->busy_set(true);
            }
          
            if (fill_buffer && !result)
            {
                _read_timeout_timer.start(1, 0);
            }

            return result;
        }

        int read_stop()
        {
            uv_read_stop(reinterpret_cast<uv_stream_t*>(this->_handle));

            if (_read_callback->busy_get())
            {
                stream_read_info* read_info = _read_callback->extra_data_get();

                _read_callback->execute(read_info->bytes_read_this_request, 0);
            }

            return 0;
        }

        void read_stop_if_read_any()
        {
            if (_read_callback->busy_get())
            {
                stream_read_info* read_info = _read_callback->extra_data_get();

                if (read_info->bytes_read_this_request > 0)
                {
                    uv_read_stop(reinterpret_cast<uv_stream_t*>(this->_handle));
                    _read_callback->execute(read_info->bytes_read_this_request, 0);
                }
            }
        }

        int write(const std::string& buf)
        {
            uv_buf_t bufs[1];

            bufs[0].base = const_cast<char*>(buf.c_str());

            bufs[0].len = buf.length();

            auto write_handle = new uv_write_t;

            write_handle->data = _write_callback.get();

            auto result = uv_write(write_handle, reinterpret_cast<uv_stream_t*>(this->_handle), bufs, 1, 
            [](uv_write_t* h, int status)
            {
                auto write_callback = reinterpret_cast<write_callback_delegate*>(h->data);

                SCOPE_EXIT(delete h);

                write_callback->execute(status);
            });

            if (result == 0)
            {
                 _read_callback->busy_set(true);
            }
            else
            {
                delete write_handle;
            }

            return result;
        }

        int shutdown()
        {
            auto shutdown_req = new uv_shutdown_t;

            shutdown_req->data = _shutdown_callback.get();

            int result = uv_shutdown(shutdown_req, reinterpret_cast<uv_stream_t*>(this->_handle),
            [](uv_shutdown_t* h, int status) 
            {
                SCOPE_EXIT(delete h);

                auto callback = reinterpret_cast<shutdown_callback_delegate*>(h->data);

                callback->execute(status);
            });

            if (result == 0)
            {
                _read_callback->busy_set(true);
            }
            else
            {
                 delete shutdown_req;
            }

            return result;
        }
    };
}}

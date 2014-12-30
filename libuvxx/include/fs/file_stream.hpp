#pragma once
#include "pplx/pplxtasks.h"
#include "streams/fileio.h"
#include "streams/astreambuf.h"
#include "streams/streams.h"
#include "streams/xxpublic.h"
#include "file.hpp"

namespace uvxx { namespace fs {

    /// <summary>
    /// A record containing the essential private data members of a file stream,
    /// in particular the parts that need to be shared between the public header
    /// file and the implementation in the implementation file.
    /// </summary>
    struct _file_info
    {
        _ASYNCRTIMP _file_info(std::ios_base::openmode mode, size_t buffer_size) : 
            m_mode(mode),
            m_memory_buffer(buffer_size)
        {
        }

        int64_t m_file_flush_pos = -1;

        int64_t m_file_read_pos = 0;
        int64_t m_file_write_pos = 0;
            
        // Positional data
        int64_t m_buffer_read_length = 0;
        int64_t m_buffer_read_pos = 0;
        int64_t m_buffer_write_pos = 0;
        bool   m_at_end = false;

        // Input buffer
        uvxx::io::memory_buffer m_memory_buffer;
        uint64_t m_file_size = 0;
        std::ios_base::openmode m_mode;
    };

    /// <summary>
    /// Private stream buffer implementation for file streams.
    /// The class itself should not be used in application code, it is used by the stream definitions farther down in the header file.
    /// </summary>
    template<typename _CharType>
    class basic_file_buffer : public uvxx::streams::details::streambuf_state_manager<_CharType>
    {
    public:
        typedef typename uvxx::streams::details::basic_streambuf<_CharType>::traits traits;
        typedef typename uvxx::streams::details::basic_streambuf<_CharType>::int_type int_type;
        typedef typename uvxx::streams::details::basic_streambuf<_CharType>::pos_type pos_type;
        typedef typename uvxx::streams::details::basic_streambuf<_CharType>::off_type off_type;

        virtual ~basic_file_buffer() 
        {
            if( this->can_read() )
            {
                this->_close_read().wait();
            }

            if (this->can_write())
            {
                this->_close_write().wait();
            }
        }

    protected:

        /// <summary>
        /// <c>can_seek</c> is used to determine whether a stream buffer supports seeking.
        /// </summary>
        virtual bool can_seek() const { return this->is_open(); }

        /// <summary>
        /// <c>has_size<c/> is used to determine whether a stream buffer supports size().
        /// </summary>
        virtual bool has_size() const { return this->is_open(); }

        virtual utility::size64_t size() const
        {
            if (!this->is_open())
                return 0;

            return m_info->m_file_size / sizeof(_CharType);
        }


        /// <summary>
        /// Gets the stream buffer size, if one has been set.
        /// </summary>
        /// <param name="direction">The direction of buffering (in or out)</param>
        /// <remarks>An implementation that does not support buffering will always return '0'.</remarks>
        virtual size_t buffer_size(std::ios_base::openmode direction = std::ios_base::in) const
        {
            if ( direction == std::ios_base::in )
                return m_info->m_memory_buffer.length_get();
            else
                return 0;
        }

        /// <summary>
        /// Sets the stream buffer implementation to buffer or not buffer.
        /// </summary>
        /// <param name="size">The size to use for internal buffering, 0 if no buffering should be done.</param>
        /// <param name="direction">The direction of buffering (in or out)</param>
        /// <remarks>An implementation that does not support buffering will silently ignore calls to this function and it will not have
        ///          any effect on what is returned by subsequent calls to buffer_size().</remarks>
        virtual void set_buffer_size(size_t size, std::ios_base::openmode direction = std::ios_base::in) 
        {
            if ( direction == std::ios_base::out ) return;

            m_info->m_memory_buffer.length_set(size);
        }

        /// <summary>
        /// For any input stream, <c>in_avail</c> returns the number of characters that are immediately available
        /// to be consumed without blocking. May be used in conjunction with <cref="::sbumpc method"/> to read data without
        /// incurring the overhead of using tasks.
        /// </summary>
        virtual size_t in_avail() const
        {
            return _in_avail_unprot();
        }

        size_t _in_avail_unprot() const
        {
            if ( !this->is_open() ) return 0;

            if ( !m_info->m_memory_buffer.length_get()) return 0;

            if (m_info->m_buffer_read_pos > m_info->m_buffer_read_length)
            {
                return 0; 
            }
            else
            {
                return static_cast<size_t>(m_info->m_buffer_read_length - m_info->m_buffer_read_pos) / sizeof(_CharType);
            }
        }
        
        pplx::task<void> _close_read_impl()
        {
            uvxx::streams::details::streambuf_state_manager<_CharType>::_close_read();

            if (this->can_write()) 
            {
                return pplx::task_from_result();
            }
            else
            {
                return m_file.close_async();
            }
        }

        pplx::task<void> _close_read()
        {
            return _close_read_impl();
        }

        pplx::task<void> _close_write()
        {
            uvxx::streams::details::streambuf_state_manager<_CharType>::_close_write();
            if (this->can_read()) 
            {
                // Read head is still open. Just flush the write data
                return flush_internal();
            }
            else
            {
                // Neither heads are open. Close the underlying device

                // We need to flush all writes if the file was opened for writing.
                return flush_internal().then([=](pplx::task<void> flushTask) -> pplx::task<void>
                {
                    // swallow exception from flush
                    try
                    {
                        flushTask.get();
                    }
                    catch(...)
                    {
                    }

                    return m_file.close_async();
                });
            }
        }

        /// <summary>
        /// Writes a single byte to an output stream.
        /// </summary>
        /// <param name="ch">The byte to write</param>
        /// <returns>A <c>task</c> that holds the value of the byte written. This is EOF if the write operation fails.</returns>
        virtual pplx::task<int_type> _putc(_CharType ch)
        {
            if (!this->is_open())
            {
                throw uvxx_exception("stream is closed");
            }

            if (m_info->m_buffer_write_pos == 0)
            {
                clear_read_buffer_before_write();
            }

            auto task_chain = pplx::task_from_result(static_cast<size_t>(0));

            if (m_info->m_buffer_write_pos >= m_info->m_memory_buffer.length_get() - 1)
            {
                task_chain = flush_write();
            }

            return task_chain.then([=](pplx::task<size_t> t)
            {
                try
                {
                    auto value = t.get();

                    m_info->m_memory_buffer.operator uint8_t*()[m_info->m_buffer_write_pos++] = ch;

                    return static_cast<int_type>(ch);
                }
                catch (uvxx_exception_with_code const&)
                {
                    return static_cast<int_type>(traits::eof());
                }
            });
        }

        /// <summary>
        /// Allocates a contiguous memory block and returns it.
        /// </summary>
        /// <param name="count">The number of characters to allocate.</param>
        /// <returns>A pointer to a block to write to, null if the stream buffer implementation does not support alloc/commit.</returns>
        _CharType* _alloc(size_t)
        {
            return nullptr; 
        }

        /// <summary>
        /// Submits a block already allocated by the stream buffer.
        /// </summary>
        /// <param name="ptr">Count of characters to be commited.</param>
        void _commit(size_t)
        {
        }

        /// <summary>
        /// Gets a pointer to the next already allocated contiguous block of data. 
        /// </summary>
        /// <param name="ptr">A reference to a pointer variable that will hold the address of the block on success.</param>
        /// <param name="count">The number of contiguous characters available at the address in 'ptr.'</param>
        /// <returns><c>true</c> if the operation succeeded, <c>false</c> otherwise.</returns>
        /// <remarks>
        /// A return of false does not necessarily indicate that a subsequent read operation would fail, only that
        /// there is no block to return immediately or that the stream buffer does not support the operation.
        /// The stream buffer may not de-allocate the block until <see cref="::release method" /> is called.
        /// If the end of the stream is reached, the function will return <c>true</c>, a null pointer, and a count of zero;
        /// a subsequent read will not succeed.
        /// </remarks>
        virtual bool acquire(_Out_ _CharType*& ptr, _Out_ size_t& count)
        {
            ptr = nullptr;
            count = 0;
            return false; 
        }

        /// <summary>
        /// Releases a block of data acquired using <see cref="::acquire method"/>. This frees the stream buffer to de-allocate the
        /// memory, if it so desires. Move the read position ahead by the count.
        /// </summary>
        /// <param name="ptr">A pointer to the block of data to be released.</param>
        /// <param name="count">The number of characters that were read.</param>
        virtual void release(_Out_writes_ (count) _CharType *, _In_ size_t count)
        {
            (void)(count);
        }

        void write_to_buffer(const _CharType *ptr, size_t& count)
        {
            size_t user_bytes = count * sizeof(_CharType);

            size_t bytes_to_write = std::min(static_cast<size_t>(m_info->m_memory_buffer.length_get() - m_info->m_buffer_write_pos), user_bytes);

            if (user_bytes == 0)
            {
                return;
            }

            memcpy(m_info->m_memory_buffer.operator char *() + m_info->m_buffer_write_pos, ptr, user_bytes);

            m_info->m_buffer_write_pos += bytes_to_write;
            count -= bytes_to_write;
        }

        void flush_read()
        {
            if (m_info->m_buffer_read_pos - m_info->m_buffer_read_length)
            {
                auto current_pos = m_file.file_position_get();
                m_file.file_position_set(static_cast<int64_t>(current_pos) + (m_info->m_buffer_read_pos - m_info->m_buffer_read_length));
            }

            m_info->m_buffer_read_pos = m_info->m_buffer_read_length = 0;
        }

        void clear_read_buffer_before_write()
        {
            if (m_info->m_buffer_read_pos == m_info->m_buffer_read_length)
            {
                m_info->m_buffer_read_pos = m_info->m_buffer_read_length = 0;
            }

            if (!this->can_seek())
            {
                throw uvxx_exception("cannot seek");
            }

            flush_read();
        }

        /// <summary>
        /// Writes a number of characters to the stream.
        /// </summary>
        /// <param name="ptr">A pointer to the block of data to be written.</param>
        /// <param name="count">The number of characters to write.</param>
        /// <returns>A <c>task</c> that holds the number of characters actually written, either 'count' or 0.</returns>
        virtual pplx::task<size_t> _putn(const _CharType *ptr, size_t count)
        {
            if (m_info->m_buffer_write_pos == 0)
            {
                clear_read_buffer_before_write();
            }

            size_t total_user_bytes = 0;

            size_t user_bytes = count * sizeof(_CharType);

            bool use_buffer = false;

            total_user_bytes = static_cast<size_t>(m_info->m_buffer_write_pos + user_bytes);

            use_buffer = (total_user_bytes + user_bytes < (size_t)(m_info->m_memory_buffer.length_get() * 2));
            
            uvxx::pplx::task<size_t> task_chain = uvxx::pplx::task_from_result(static_cast<size_t>(0));

            if (m_info->m_file_flush_pos != -1)
            {
                task_chain.then([=](size_t)
                {
                    return flush_write();
                });
            }

            if (use_buffer)
            {
                write_to_buffer(ptr, user_bytes);

                if (m_info->m_buffer_write_pos < m_info->m_memory_buffer.length_get())
                {
                    return task_chain.then([user_bytes](size_t)
                    {
                        return user_bytes;
                    });
                }

                return task_chain.then([=](size_t)
                {
                    auto write_position = m_info->m_file_write_pos == -1 ? -1 : m_info->m_file_write_pos * sizeof(_CharType);

                    return m_file.write_async(m_info->m_memory_buffer,
                                       static_cast<size_t>(0),
                                       static_cast<size_t>(m_info->m_buffer_write_pos),
                                       write_position);
                }).
                then([=](size_t bytes_wrote) mutable
                {
                    m_info->m_buffer_write_pos = 0;

                    write_to_buffer(ptr, user_bytes);

                    return bytes_wrote;
                });
            }
            else
            {
                if (m_info->m_buffer_write_pos > 0)
                {
                    task_chain.then([=](size_t)
                    {
                        auto write_position = m_info->m_file_write_pos == -1 ? -1 : m_info->m_file_write_pos * sizeof(_CharType);

                        return m_file.write_async(m_info->m_memory_buffer,
                                                  static_cast<size_t>(0), 
                                                  static_cast<size_t>(m_info->m_buffer_write_pos) * sizeof(_CharType));
                    }).
                    then([=](uvxx::pplx::task<size_t> t)
                    {
                        auto bytes_wrote = t.get();

                        m_info->m_buffer_write_pos = 0;

                        if (m_info->m_file_write_pos != -1)
                        {
                            m_info->m_file_write_pos += (bytes_wrote / sizeof(_CharType));
                        }

                        return bytes_wrote;
                    });
                }

                return task_chain.then([=](size_t bytes) mutable
                {
                     auto write_position = m_info->m_file_write_pos == -1 ? -1 : m_info->m_file_write_pos * sizeof(_CharType);

                     return m_file.write_async(reinterpret_cast<const uint8_t*>(const_cast<_CharType*>(ptr)), 
                                               m_info->m_memory_buffer.length_get(),
                                               static_cast<size_t>(0), 
                                               user_bytes, 
                                               write_position);
                }).
                then([=](pplx::task<size_t> t)
                {
                    try
                    {
                        auto bytes_wrote = t.get();

                        m_info->m_file_write_pos += (bytes_wrote / sizeof(_CharType));

                        return bytes_wrote;
                    }
                    catch (uvxx_exception const&)
                    {
                    	return static_cast<size_t>(0);
                    }
                    catch (...)
                    {
                        throw;
                    }
                });
            }
        }

        /// <summary>
        /// Reads a single byte from the stream and advance the read position.
        /// </summary>
        /// <returns>A <c>task</c> that holds the value of the byte read. This is EOF if the read fails.</returns>
        virtual pplx::task<int_type> _bumpc()
        {
            auto task_chain = pplx::task_from_result(static_cast<size_t>(0));

            if (m_info->m_buffer_read_pos == m_info->m_buffer_read_length || m_info->m_buffer_read_length == 0)
            {
                if (m_info->m_buffer_write_pos > 0)
                {
                    task_chain = flush_write();
                }

                task_chain = m_file.read_async(m_info->m_memory_buffer, 
                                               0, 
                                               m_info->m_memory_buffer.length_get(), 
                                               m_info->m_file_read_pos * sizeof(_CharType)).
                then([=](uvxx::pplx::task<size_t> t)
                {
                    size_t bytes_read = t.get();

                    m_info->m_buffer_read_length = bytes_read;

                    m_info->m_file_read_pos += (bytes_read / sizeof(_CharType));

                    m_info->m_buffer_read_pos = 0;

                    return bytes_read;
                });
            }

            return task_chain.then([=](pplx::task<size_t> t)
            {
                try
                {
                    t.get();
                }
                catch (...)
                {
                    return static_cast<int_type>(traits::eof());
                }

                if (m_info->m_buffer_read_pos == m_info->m_buffer_read_length && 
                   !m_info->m_buffer_read_pos)
                {
                    return static_cast<int_type>(traits::eof());
                }

                int_type ret_val = m_info->m_memory_buffer.operator uint8_t *()[m_info->m_buffer_read_pos++];

                return static_cast<int_type>(ret_val);
            });
        }

        /// <summary>
        /// Reads a single byte from the stream and advance the read position.
        /// </summary>
        /// <returns>The value of the byte. EOF if the read fails. <see cref="::requires_async method" /> if an asynchronous read is required</returns>
        /// <remarks>This is a synchronous operation, but is guaranteed to never block.</remarks>
        virtual int_type _sbumpc() 
        {
            if ( m_info->m_at_end ) return traits::eof();

            if ( _in_avail_unprot() == 0 ) return traits::requires_async();

            _CharType ch = m_info->m_memory_buffer.operator uint8_t *()[m_info->m_buffer_read_pos * sizeof(_CharType)];

            m_info->m_buffer_read_pos += sizeof(_CharType);

            return ch;
        }


        /// <summary>
        /// Reads a single byte from the stream without advancing the read position.
        /// </summary>
        /// <returns>The value of the byte. EOF if the read fails.</returns>
        pplx::task<int_type> _getc()
        {
            /* maybe reuse this buffer? */
            auto buff = std::make_shared<_CharType>();

            return _getn(buff.get(), 1).
            then([=](pplx::task<size_t> t)
            {
                _CharType result = *buff.get();

                try
                {
                    auto result = t.get();

                    m_info->m_buffer_read_pos -= sizeof(_CharType);

                    return static_cast<int_type>(result);
                }
                catch (uvxx_exception_with_code const&)
                {
                    return static_cast<int_type>(traits::eof());
                }
            });
        }

        /// <summary>
        /// Reads a single byte from the stream without advancing the read position.
        /// </summary>
        /// <returns>The value of the byte. EOF if the read fails. <see cref="::requires_async method" /> if an asynchronous read is required</returns>
        /// <remarks>This is a synchronous operation, but is guaranteed to never block.</remarks>
        int_type _sgetc()
        {
            if (m_info->m_at_end) return traits::eof();

            if (_in_avail_unprot() == 0) return traits::requires_async();

            _CharType ch = m_info->m_memory_buffer.operator uint8_t *()[m_info->m_buffer_read_pos * sizeof(_CharType)];

            return ch;
        }

        /// <summary>
        /// Advances the read position, then return the next character withouth advancing again.
        /// </summary>
        /// <returns>A <c>task</c> that holds the value of the byte, which is EOF if the read fails.</returns>
        virtual pplx::task<int_type> _nextc()
        {
            auto char_buff = std::make_shared<int_type>(traits::eof());

            this->seekoff(1, std::ios_base::cur, std::ios_base::in);

            return _getn(static_cast<_CharType*>(static_cast<void*>(char_buff.get())), 1)
            .then([=](pplx::task<size_t> t)
            {
                size_t bytes_read = 0;

                if (m_info->m_at_end)
                {
                    return traits::eof();
                }

                try
                {
                    bytes_read = t.get();

                    if (bytes_read == 0 || bytes_read == traits::eof())
                    {
                        return traits::eof();
                    }

                    seekoff(-1, std::ios_base::cur, std::ios_base::in);

                    return *char_buff.get();
                }
                catch (uvxx_exception_with_code const&)
                {
                	return traits::eof();
                }
            });
        }

        /// <summary>
        /// Retreats the read position, then return the current character withouth advancing.
        /// </summary>
        /// <returns>A <c>task</c> that holds the value of the byte. The value is EOF if the read fails, <c>requires_async</c> if an asynchronous read is required</returns>
        virtual pplx::task<int_type> _ungetc() 
        {
            auto char_buff = std::make_shared<int_type>(traits::eof());

            seekoff(-1, std::ios_base::cur, std::ios_base::in);

            return _getn(static_cast<_CharType*>(static_cast<void*>(char_buff.get())), 1)
            .then([=](pplx::task<size_t> t)
            {
                size_t bytes_read = 0;

                if (m_info->m_at_end)
                {
                    return traits::eof();
                }

                try
                {
                    bytes_read = t.get();

                    if (bytes_read == 0 || bytes_read == traits::eof())
                    {
                        return traits::eof();
                    }

                    seekoff(-1, std::ios_base::cur, std::ios_base::in);

                    return *char_buff.get();
                }
                catch (uvxx_exception_with_code const&)
                {
                	return traits::eof();
                }
            });
        }

        size_t read_from_buffer(_CharType *ptr, size_t count)
        {
            int64_t read_bytes = m_info->m_buffer_read_length - m_info->m_buffer_read_pos;

            if (read_bytes == 0)
            {
                return 0;
            }

            if (read_bytes > count)
            {
                read_bytes = count;
            }

            memcpy(ptr, m_info->m_memory_buffer.operator char *() + m_info->m_buffer_read_pos, static_cast<size_t>(read_bytes));

            m_info->m_buffer_read_pos += static_cast<size_t>(read_bytes);

            return static_cast<size_t>(read_bytes);
        }

        pplx::task<size_t> flush_write()
        {
            auto current_position = m_info->m_file_flush_pos == -1 ? m_info->m_file_write_pos : m_info->m_file_flush_pos;
            
            auto write_position = current_position == -1 ? -1 : current_position * sizeof(_CharType);

            return m_file.write_async(m_info->m_memory_buffer, 
                                      0, 
                                      static_cast<size_t>(m_info->m_buffer_write_pos), 
                                      current_position * static_cast<int64_t>(sizeof(_CharType))).
            then([=](uvxx::pplx::task<size_t> t)
            {
                auto bytes_wrote = t.get();

                if (m_info->m_file_write_pos != -1 && m_info->m_file_flush_pos != -1)
                {
                    m_info->m_file_write_pos += (bytes_wrote / sizeof(_CharType));
                }
                else if (m_info->m_file_flush_pos != -1)
                {
                    m_info->m_file_flush_pos = -1;
                }

                return bytes_wrote;
            });
        }

        /// <summary>
        /// Reads up to a given number of characters from the stream.
        /// </summary>
        /// <param name="ptr">The address of the target memory area</param>
        /// <param name="count">The maximum number of characters to read</param>
        /// <returns>A <c>task</c> that holds the number of characters read. This number is O if the end of the stream is reached, EOF if there is some error.</returns>
        virtual pplx::task<size_t> _getn(_Out_writes_ (count) _CharType *ptr, _In_ size_t count)
        {
            size_t user_byte_count = count * sizeof(_CharType);

            size_t bytes_from_buffer = read_from_buffer(ptr, user_byte_count);

            if (bytes_from_buffer == user_byte_count)
            {
                return uvxx::pplx::task_from_result(bytes_from_buffer);
            }

            int64_t already_satisfied = bytes_from_buffer;

            if (bytes_from_buffer > 0)
            {
                user_byte_count -= bytes_from_buffer;
            }

            m_info->m_buffer_read_length = m_info->m_buffer_read_pos = 0;

            auto task_chain = uvxx::pplx::task_from_result(static_cast<size_t>(0));

            if (m_info->m_buffer_write_pos > 0)
            {
                task_chain = flush_write();
            }

            if (user_byte_count >= m_info->m_memory_buffer.length_get())
            {
                task_chain.then([=](size_t bytes)
                {
                    return m_file.read_async(m_info->m_memory_buffer, 
                                             0, 
                                             user_byte_count, 
                                             m_info->m_file_read_pos * sizeof(_CharType));
                }).
                then([=](uvxx::pplx::task<size_t> t)
                {
                    auto bytes_read = t.get();

                    m_info->m_file_read_pos += (bytes_read / sizeof(_CharType));

                    memcpy(ptr + user_byte_count, m_info->m_memory_buffer.operator char *(), bytes_read);

                    return static_cast<size_t>(bytes_read + already_satisfied);
                });
            }

            return task_chain.then([=](size_t bytes)
            {
                return m_file.read_async(m_info->m_memory_buffer, 
                                         0, 
                                         m_info->m_memory_buffer.length_get(), 
                                         m_info->m_file_read_pos * sizeof(_CharType));
            }).
            then([=](uvxx::pplx::task<size_t> t) mutable
            {
                size_t bytes_read = 0;
                
                try
                {
                    bytes_read = t.get();
                }
                catch (uv_exception_with_code const&)
                {
                    return static_cast<size_t>(traits::eof());
                }

                m_info->m_buffer_read_length = bytes_read;

                m_info->m_file_read_pos += (bytes_read / sizeof(_CharType));

                bytes_from_buffer = read_from_buffer(ptr, user_byte_count);

                return static_cast<size_t>(bytes_from_buffer + already_satisfied);
            });
        }

        /// <summary>
        /// Reads up to a given number of characters from the stream.
        /// </summary>
        /// <param name="ptr">The address of the target memory area</param>
        /// <param name="count">The maximum number of characters to read</param>
        /// <returns>The number of characters read. O if the end of the stream is reached or an asynchronous read is required.</returns>
        /// <remarks>This is a synchronous operation, but is guaranteed to never block.</remarks>
        size_t _sgetn(_Out_writes_ (count) _CharType *ptr, _In_ size_t count)
        {
            if ( count == 0 || in_avail() == 0 ) return 0;

            auto user_count = count * sizeof(_CharType);

            auto bytes_read = read_from_buffer(ptr, user_count);

            m_info->m_at_end = (bytes_read < user_count);

            return bytes_read;
        }

        /// <summary>
        /// Copies up to a given number of characters from the stream.
        /// </summary>
        /// <param name="ptr">The address of the target memory area</param>
        /// <param name="count">The maximum number of characters to copy</param>
        /// <returns>The number of characters copied. O if the end of the stream is reached or an asynchronous read is required.</returns>
        /// <remarks>This is a synchronous operation, but is guaranteed to never block.</remarks>
        virtual size_t _scopy(_Out_writes_ (count) _CharType *ptr, _In_ size_t count)
        {
            return 0;
        }

        /// <summary>
        /// Gets the current read or write position in the stream.
        /// </summary>
        /// <param name="direction">The I/O direction to seek (see remarks)</param>
        /// <returns>The current position. EOF if the operation fails.</returns>
        /// <remarks>Some streams may have separate write and read cursors. 
        ///          For such streams, the direction parameter defines whether to move the read or the write cursor.</remarks>
        virtual pos_type getpos(std::ios_base::openmode mode) const
        {
            return const_cast<basic_file_buffer*>(this)->seekoff(0, std::ios_base::cur, mode);
        }

        /// <summary>
        /// Seeks to the given position.
        /// </summary>
        /// <param name="pos">The offset from the beginning of the stream</param>
        /// <param name="direction">The I/O direction to seek (see remarks)</param>
        /// <returns>The position. EOF if the operation fails.</returns>
        /// <remarks>Some streams may have separate write and read cursors. 
        ///          For such streams, the direction parameter defines whether to move the read or the write cursor.</remarks>
        virtual pos_type seekpos(pos_type pos, std::ios_base::openmode mode) 
        {
            if ( mode == std::ios_base::in ) 
            {
                int64_t old_position = m_info->m_file_read_pos - (m_info->m_buffer_read_length - m_info->m_buffer_read_pos);
                int64_t new_position = old_position + (pos - old_position);

                m_info->m_buffer_read_pos = (new_position - (old_position - m_info->m_buffer_read_pos));

                if (0 <= m_info->m_buffer_read_pos && m_info->m_buffer_read_pos < m_info->m_buffer_read_length)
                {
                    m_info->m_file_read_pos = (new_position + (m_info->m_buffer_read_length - m_info->m_buffer_read_pos));
                }
                else 
                {  
                    m_info->m_buffer_read_pos = m_info->m_buffer_read_length = 0;
                    m_info->m_file_read_pos = pos;
                }

                return pos;
            }
            else if ( (m_info->m_mode & std::ios::ios_base::app) == 0 )
            {
                if (m_info->m_buffer_write_pos > 0 && m_info->m_file_flush_pos == -1)
                {
                    m_info->m_file_flush_pos = m_info->m_file_write_pos;
                }

                return m_info->m_file_write_pos = pos;
            }

            return (pos_type)std::char_traits<_CharType>::eof(); 
        }

        /// <summary>
        /// Seeks to a position given by a relative offset.
        /// </summary>
        /// <param name="offset">The relative position to seek to</param>
        /// <param name="way">The starting point (beginning, end, current) for the seek.</param>
        /// <param name="mode">The I/O direction to seek (see remarks)</param>
        /// <returns>The position. EOF if the operation fails.</returns>
        /// <remarks>Some streams may have separate write and read cursors. 
        ///          For such streams, the mode parameter defines whether to move the read or the write cursor.</remarks>
        virtual pos_type seekoff(off_type offset, std::ios_base::seekdir way, std::ios_base::openmode mode) 
        {
            if ( mode == std::ios_base::in ) 
            {
                int64_t current_pos = m_info->m_file_read_pos - (m_info->m_buffer_read_length - m_info->m_buffer_read_pos);

                switch ( way )
                {
                case std::ios_base::beg:
                    return seekpos(offset, mode);

                case std::ios_base::cur:
                    return seekpos(current_pos + offset, mode);

                case std::ios_base::end:
                   // return (pos_type)_seekrdtoend_fsb(m_info, int64_t(offset), sizeof(_CharType));
                    break;
                }
            }
            else if ( (m_info->m_mode & std::ios::ios_base::app) == 0 )
            {
                switch ( way )
                {
                case std::ios_base::beg:
                    return seekpos(offset, mode);

                case std::ios_base::cur:
                    return seekpos(offset + m_info->m_file_write_pos, mode);

                case std::ios_base::end:
                    return seekpos(size_t(-1), mode);

                    break;
                }
            }
            return (pos_type)traits::eof(); 
        }

        /// <summary>
        /// For output streams, flush any internally buffered data to the underlying medium.
        /// </summary>
        virtual pplx::task<bool> _sync()
        {
            return flush_internal().then([](){return true;});
        }

    private:
        template<typename _CharType1> friend class file_buffer;

        pplx::task<void> flush_internal()
        {
            if (m_info->m_buffer_write_pos > 0)
            {
                return flush_write().then([](size_t){});
            }

            uvxx::pplx::task<size_t> task_chain = uvxx::pplx::task_from_result(static_cast<size_t>(0));

            if (m_info->m_buffer_read_pos < m_info->m_buffer_read_length)
            {
                if (!can_seek())
                {
                    return task_chain.then([](size_t){});
                }
                
                flush_read();

                return task_chain.then([](size_t){});
            }

            m_info->m_buffer_write_pos = m_info->m_buffer_read_pos = m_info->m_buffer_read_length = 0;

            return task_chain.then([](size_t){});
        }

        basic_file_buffer(std::unique_ptr<_file_info> info, file _file) : m_info(std::move(info)), 
                                                                          m_file(_file), 
                                                                          uvxx::streams::details::streambuf_state_manager<_CharType>(info->m_mode)
        { 

        }

        static pplx::task<std::shared_ptr<uvxx::streams::details::basic_streambuf<_CharType>>> open(
            const utility::string_t &_Filename,
            std::ios_base::openmode _Mode = std::ios_base::out)
        {
            uvxx::fs::file _file;

            return _file.open_async(_Filename, _Mode).
            then([=](uvxx::pplx::task<void> t)
            {
                t.get();

                return uvxx::fs::file::get_file_info_async(_Filename);
            }).
            then([=](uvxx::pplx::task<file_info> t) mutable -> std::shared_ptr<uvxx::streams::details::basic_streambuf<_CharType>>
            {

                file_info fileinfo;
                try
                {
                    fileinfo = t.get();
                }
                catch (uv_exception_with_code const&)
                {
                    if (_Mode & std::ios_base::in)
                    {
                        throw;
                    }
                }

                auto info = std::unique_ptr<_file_info>(new _file_info(_Mode, 1024 * 1024));

                info->m_file_size = fileinfo.length_get();

                if ( _Mode & std::ios_base::app || _Mode & std::ios_base::ate )
                {
                    info->m_file_write_pos = static_cast<size_t>(-1); // Start at the end of the file.
                }

                auto buff = new basic_file_buffer<_CharType>(std::move(info), _file);

                auto file_buff = std::shared_ptr<basic_file_buffer<_CharType>>(buff);

                return std::static_pointer_cast<uvxx::streams::details::basic_streambuf<_CharType>>(file_buff);
            });
        }

        uvxx::fs::file m_file;

        std::unique_ptr<_file_info> m_info;
    };


    /// <summary>
    /// Stream buffer for file streams.
    /// </summary>
    /// <typeparam name="_CharType">
    /// The data type of the basic element of the <c>file_buffer</c>.
    /// </typeparam>
    template<typename _CharType>
    class file_buffer
    {
    public:
        /// <summary>
        /// Open a new stream buffer representing the given file.
        /// </summary>
        /// <param name="file_name">The name of the file</param>
        /// <param name="mode">The opening mode of the file</param>
        /// <param name="prot">The file protection mode</param>
        /// <returns>A <c>task</c> that returns an opened stream buffer on completion.</returns>
        static pplx::task<uvxx::streams::streambuf<_CharType>> open(
            const utility::string_t &file_name,
            std::ios_base::openmode mode = std::ios_base::out
            )
        {
            auto bfb = basic_file_buffer<_CharType>::open(file_name, mode);
            return bfb.then([](pplx::task<std::shared_ptr<uvxx::streams::details::basic_streambuf<_CharType>>> op)  mutable -> uvxx::streams::streambuf<_CharType>
                            {
                                return uvxx::streams::streambuf<_CharType>(op.get());
                            },uvxx::pplx::task_continuation_context::use_current());
        }
    };


    /// <summary>
    /// File stream class containing factory functions for file streams.
    /// </summary>
    /// <typeparam name="_CharType">
    /// The data type of the basic element of the <c>file_stream</c>.
    /// </typeparam>
    template<typename _CharType>
    class file_stream
    {
    public:

        /// <summary>
        /// Open a new input stream representing the given file.
        /// The file should already exist on disk, or an exception will be thrown.
        /// </summary>
        /// <param name="file_name">The name of the file</param>
        /// <param name="mode">The opening mode of the file</param>
        /// <param name="prot">The file protection mode</param>
        /// <returns>A <c>task</c> that returns an opened input stream on completion.</returns>
        static pplx::task<streams::basic_istream<_CharType>> open_istream(
            const utility::string_t &file_name,
            std::ios_base::openmode mode = std::ios_base::in
            )
        {
            mode |= std::ios_base::in;
            return file_buffer<_CharType>::open(file_name, mode)
                .then([](streams::streambuf<_CharType> buf) -> uvxx::streams::basic_istream<_CharType>
            {
                return uvxx::streams::basic_istream<_CharType>(buf);
            });
        }

        /// <summary>
        /// Open a new ouput stream representing the given file.
        /// If the file does not exist, it will be create unless the folder or directory
        /// where it is to be found also does not exist.
        /// </summary>
        /// <param name="file_name">The name of the file</param>
        /// <param name="mode">The opening mode of the file</param>
        /// <param name="prot">The file protection mode</param>
        /// <returns>A <c>task</c> that returns an opened output stream on completion.</returns>
        static pplx::task<streams::basic_ostream<_CharType>> open_ostream(
            const utility::string_t &file_name,
            std::ios_base::openmode mode = std::ios_base::out)
        {
            mode |= std::ios_base::out;

            return file_buffer<_CharType>::open(file_name, mode)
            .then([](streams::streambuf<_CharType> buf) -> uvxx::streams::basic_ostream<_CharType>
            {
                return uvxx::streams::basic_ostream<_CharType>(buf);
            });
        }

        typedef file_stream<uint8_t> fstream;
    };
}}
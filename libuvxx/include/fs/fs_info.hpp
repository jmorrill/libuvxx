#pragma once
#include <chrono>
#include <ctime>
#include <string>

namespace uvxx { namespace fs 
{
    class fs_info
    {
        std::time_t _creation_time;
        std::time_t _last_access_time;
        std::time_t _last_write_time;
        std::string _full_name;

    public:
        fs_info(std::time_t creation_time, 
                std::time_t last_access_time, 
                std::time_t last_write_time, 
                std::string const& full_name) : _creation_time(creation_time), 
                                                _last_access_time(last_access_time),
                                                _last_write_time(last_write_time),
                                                _full_name(full_name)
        {

        }

        fs_info(fs_info const&) = default;

        fs_info& operator=(fs_info const&) = default;

        fs_info(fs_info&& rhs) : _full_name(std::move(rhs._full_name))
        {
            _creation_time = rhs._creation_time;
            _last_write_time = rhs._last_write_time;
            _last_access_time = rhs._last_access_time;
        }

        fs_info& operator=(fs_info&& rhs)
        {
            _creation_time = rhs._creation_time;
            _last_write_time = rhs._last_write_time;
            _last_access_time = rhs._last_access_time;
            _full_name = std::move(rhs._full_name);
            return *this;
        }

        std::time_t creation_time_get() const
        {
            return _creation_time;
        }

        std::time_t last_access_time_get() const
        {
            return _last_write_time;
        }

        std::time_t last_write_time_get() const
        {
            return _last_write_time;
        }

        std::string const & full_name_get() const
        {
            return _full_name;
        }
    };

    class file_info : fs_info
    {
        int64_t _length;

    public:
        file_info(int64_t     length, 
                  std::time_t creation_time, 
                  std::time_t last_access_time,
                  std::time_t last_write_time, 
                  std::string const& full_name) : fs_info(creation_time, 
                                                          last_access_time,
                                                          last_write_time,
                                                          full_name), _length(length)

        {
            
        }

        file_info() : fs_info(0,
            0,
            0,
            ""), _length(0)
        {

        }

        file_info(file_info const&) = default;

        file_info& operator=(file_info const&) = default;

        file_info(file_info&& rhs) : fs_info(rhs)
        {

        }

        fs_info& operator=(fs_info&& rhs)
        {
             return static_cast<fs_info&>(fs_info::operator=(std::move(rhs))); 
        }

        int64_t length_get()
        {
            return _length;
        }
    };
}}
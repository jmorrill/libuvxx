#pragma once
#include "uvxx_exception.hpp"

namespace uvxx { namespace rtsp 
{ 
    class rtsp_exception : public uvxx_exception
    {
    public:
        rtsp_exception(const std::string & message) : uvxx_exception(message)
        {

        }
    };

    class rtsp_transport_exception : public rtsp_exception
    {
    public:
        rtsp_transport_exception(int code, const std::string & message) : rtsp_exception(message),
            _code(code)
        {

        }

        explicit operator bool() const
        {
            return _code != 0;
        }

        int code()
        {
            return _code;
        }

    private:
        int _code;
    };

    class rtsp_network_exception : public rtsp_exception
    {
    public:
        rtsp_network_exception(int code, const std::string & message) : rtsp_exception(message),
            _code(code)
        {

        }

        explicit operator bool() const
        {
            return _code != 0;
        }

        int code()
        {
            return _code;
        }

    private:
        int _code;
    };
}}
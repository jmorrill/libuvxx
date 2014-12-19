#pragma once
#include "_uvxx_internal.hpp"
#include "_uv_stream.hpp"
#include "_uv_loop.hpp"
#include "_uvxx_loop_callback.hpp"

namespace uvxx { namespace details
{
    class _uv_tcp : public _uv_stream <uv_tcp_t>
    {
    public:
        using connect_callback_t = std::function<void(int)>;

    public:
        _uv_tcp(_uv_loop* l);

        virtual ~_uv_tcp();

        void set_connect_callback(connect_callback_t connect_callback);

        int connect(const std::string& ip, int port);

        int connect6(const std::string& ip, int port);

        bool nodelay(bool enable);

        bool keepalive(bool enable, unsigned int delay);

        bool simultanious_accepts(bool enable);

    private:
        using connect_callback_delegate_t = _uvxx_loop_callback<connect_callback_t>;
        using connect_callback_delegate_ptr = callback_unique_ptr<connect_callback_delegate_t>;

    private:
        connect_callback_delegate_ptr _connect_callback;

        _uv_loop* _loop;
    };
}}

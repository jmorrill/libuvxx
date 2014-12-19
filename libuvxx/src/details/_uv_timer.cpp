#include "details/_uv_loop.hpp"
#include "details/_uv_timer.hpp"
using namespace std;

namespace uvxx { namespace details
{
    void _uv_timer::timer_callback(uv_timer_t *req)
    {
        auto t = reinterpret_cast<_uv_timer*>(req->data);

        t->_callback();
    }

    _uv_timer::_uv_timer(details::_uv_loop* loop, function<void()> callback) : 
        _callback(std::move(callback))
    {
        _loop = loop;
        uv_timer_init(*loop, *this);
    }

    bool _uv_timer::start(uint64_t timeout, uint64_t repeat)
    {
        return 0 == uv_timer_start(*this, timer_callback, timeout, repeat);
    }

    bool _uv_timer::stop()
    {
        return 0 == uv_timer_stop(*this);
    }
}}

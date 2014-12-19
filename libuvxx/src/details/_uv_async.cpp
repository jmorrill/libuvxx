#include "details/_uv_loop.hpp"
#include "details/_uv_async.hpp"

namespace uvxx { namespace details
{
    void _uv_async::async_callback(uv_async_t* handle)
    {
        auto pasync = static_cast<_uv_async*>(handle->data);

        pasync->_callback();
    }

    _uv_async::_uv_async(_uv_loop* uv_loop, std::function<void()> callback) : 
        _uv_handle<uv_async_t>(), 
        _callback(std::move(callback))
    {
        static_cast<uv_async_t*>(*this)->loop = nullptr;
        uv_async_init(*uv_loop, *this, async_callback);
    }

    bool _uv_async::send()
    {
        if (static_cast<uv_async_t*>(*this) == nullptr)
        {
            return false;
        }

	    if (static_cast<uv_async_t*>(*this)->loop)
        {
		    return 0 == uv_async_send(*this);
	    } 
        else
        {
		    return false;
	    }
    }
}}
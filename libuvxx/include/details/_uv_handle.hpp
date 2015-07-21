#pragma once
#include "_uvxx_internal.hpp"
#include <functional>

namespace uvxx { namespace details
{
    class _uv_loop;
}}

namespace uvxx { namespace details
{
    template<typename T>
    class _uv_handle 
    {
    public:
	    using handle_type = _uv_handle ;

	    explicit _uv_handle() 
        {
            _handle = new T;
		    this->_handle->data = this;
	    }

	    _uv_handle(const _uv_handle&) = delete;

	    _uv_handle& operator=(const _uv_handle&) = delete;

	    operator _uv_loop*() 
        {
            return reinterpret_cast<_uv_loop*>(this->_handle->loop->data); 
        }

        _uv_loop* loop_get()
        {
            return reinterpret_cast<_uv_loop*>(this->_handle->loop->data); 
        }

	    operator uv_loop_t*() { return this->_handle->loop; }

	    operator uv_handle_t*() { return reinterpret_cast<uv_handle_t*>(this->_handle); }

	    operator const _uv_loop*() const { return reinterpret_cast<const _uv_loop*>(this->_handle->loop->data); }

	    operator const uv_loop_t*() const { return this->_handle->loop; }

	    operator const uv_handle_t*() const { return reinterpret_cast<const uv_handle_t*>(this->_handle); }

	    template<typename U = T, typename = typename std::enable_if<!std::is_same<U, uv_handle_t>::value>::type>
	    operator T*() { return this->_handle; }

	    template<typename U = T, typename = typename std::enable_if<!std::is_same<U, uv_handle_t>::value>::type>
	    operator const T*() const { return this->_handle; }

        template<typename HANDLE>
        T* get()
        {
            return reinterpret_cast<HANDLE*>(_handle);
        }

        template<typename HANDLE>
        const T* get() const
        {
            return reinterpret_cast<const HANDLE*>(_handle);
        }
	    //template<class T1, class T2>
	    //friend bool operator==(const uvxx::details::handle<T1>& lhs, const uvxx::details::handle<T2>& rhs) _noexcept {
	    //	return &lhs._handle == &rhs._handle;
	    //}

	    //template<class T1, class T2>
	    //friend bool operator!=(const uvxx::details::handle<T1>& lhs, const uvxx::details::handle<T2>& rhs) _noexcept {
	    //	return &lhs._handle != &rhs._handle;
	    //}

	    void close() 
        {
            if (_handle == nullptr)
            {
                return;
            }

		    if (!uv_is_closing(*this)) 
            {
			    uv_close(*this, [](uv_handle_t* handle) 
                {
                    delete reinterpret_cast<T*>(handle);
			    });
		    }
            else
            {
                delete reinterpret_cast<T*>(_handle);
            }

             _handle = nullptr;
	    }

        bool is_active()
        {
            return uv_is_active(get()) != 0;
        }

	    void ref() 
        {
		    uv_ref(*this);
	    }

	    void unref() 
        {
		    uv_unref(*this);
	    }

    protected:
	    T* _handle;
    };
}}

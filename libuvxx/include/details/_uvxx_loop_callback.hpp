#pragma once
#include <memory>
namespace uvxx { namespace details
{
    template<typename T>
    using callback_unique_ptr = std::unique_ptr < T, std::function<void(T*)> > ;

    template<typename FUNCTION_T>
    class _uvxx_loop_callback
    {
        FUNCTION_T _callback;
        bool _abort = false;
        bool _busy = false;
        bool _closed = false;
    protected:
        template<typename T>
        inline static callback_unique_ptr<T> create_impl()
        {
            callback_unique_ptr<T> callback(new T(), [](T * p)
            {
                p->close();
            });

            return callback;
        }

    public:
        _uvxx_loop_callback() = default;

        virtual ~_uvxx_loop_callback() = default;

        _uvxx_loop_callback(const _uvxx_loop_callback&) = delete;

        _uvxx_loop_callback& operator=(const _uvxx_loop_callback&) = delete;

        static callback_unique_ptr<_uvxx_loop_callback<FUNCTION_T>>  create()
        {
            return create_impl<_uvxx_loop_callback<FUNCTION_T>>();
        }

        void close()
        {
            if (_busy)
            {
                abort_set(true);
                _closed = true;
            }
            else
            {
                delete this;
            }
        }

        void abort_set(bool abort)
        {
            _abort = abort;
        }

        bool abort_get()
        {
            return _abort;
        }

        void busy_set(bool busy)
        {
            _busy = busy;
        }

        bool busy_get()
        {
           return _busy;
        }

        void callback_set(FUNCTION_T callback)
        {
            _callback = std::move(callback);
        }

        template<typename...TARGS>
        bool operator()(TARGS&&...args)
        {
            return execute(std::forward<TARGS>(args)...);
        }

        template<typename...TARGS>
        bool execute(TARGS&&...args)
        {
            busy_set(false);

            if (_closed)
            {
                close();
                return false;
            }

            if (_abort)
            {
                return false;
            }

            _callback(std::forward<TARGS>(args)...);

            return true;
        }
    };

    template<typename FUNCTION_T, typename EXTRA_DATA_T>
    class _extra_data_uvxx_loop_callback : public _uvxx_loop_callback<FUNCTION_T>
    {
        std::unique_ptr<EXTRA_DATA_T> _extra_data;

        using this_type = _extra_data_uvxx_loop_callback<FUNCTION_T, EXTRA_DATA_T>;
    public:
        _extra_data_uvxx_loop_callback() 
        {

        }

        virtual ~_extra_data_uvxx_loop_callback()
        {

        }

        static callback_unique_ptr<this_type> create()
        {
            return _uvxx_loop_callback<FUNCTION_T>::template create_impl<this_type>();
        }

        EXTRA_DATA_T* extra_data_get()
        {
            return _extra_data.get();
        }

        void extra_data_set(std::unique_ptr<EXTRA_DATA_T> val)
        {
            _extra_data = std::move(val);
        }
    };
}}
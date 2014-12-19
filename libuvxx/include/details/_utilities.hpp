#pragma once
#include <atomic>
#include <cassert>
#include <memory>
#include <utility>
#include "pplx/pplx.h"

namespace uvxx { namespace details
{
    template <typename F>
    struct ScopeExit {
        ScopeExit(F f) : f(f) {}
        ~ScopeExit() { f(); }
        F f;
    };

    template <typename F>
    ScopeExit<F> MakeScopeExit(F f) {
        return ScopeExit<F>(f);
    };

#define STRING_JOIN2(arg1, arg2) DO_STRING_JOIN2(arg1, arg2)
#define DO_STRING_JOIN2(arg1, arg2) arg1 ## arg2
#define SCOPE_EXIT(code) \
auto STRING_JOIN2(scope_exit_, __LINE__) = MakeScopeExit([=](){code;})

    class spin_lock
    {
    public:

        spin_lock()
            : _lock(0l)
        {
        }

        void lock()
        {
            if ( uvxx::pplx::details::atomic_compare_exchange(_lock, 1l, 0l) != 0l )
            {
                do
                {
                    uvxx::pplx::details::platform::YieldExecution();

                } while ( uvxx::pplx::details::atomic_compare_exchange(_lock, 1l, 0l) != 0l );
            }
        }

        void unlock()
        {
            // fence for release semantics
            uvxx::pplx::details::atomic_exchange(_lock, 0l);
        }

    private:
        uvxx::pplx::details::atomic_long _lock;
    };

    typedef uvxx::pplx::scoped_lock<spin_lock> scoped_spin_lock;

    template <typename T, typename F>
    class capture_impl
    {
        T x;
        F f;
    public:
        capture_impl(T && x, F && f)
            : x{ std::forward<T>(x) }, f{ std::forward<F>(f) }
        {}

        template <typename ...Ts> auto operator()(Ts&&...args)
            -> decltype(f(x, std::forward<Ts>(args)...))
        {
            return f(x, std::forward<Ts>(args)...);
        }

        template <typename ...Ts> auto operator()(Ts&&...args) const
            -> decltype(f(x, std::forward<Ts>(args)...))
        {
            return f(x, std::forward<Ts>(args)...);
        }
    };

    template <typename T, typename F>
    capture_impl<T, F> capture(T && x, F && f)
    {
        return capture_impl<T, F>(
            std::forward<T>(x), std::forward<F>(f));
    }

}}
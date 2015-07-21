#pragma once
#include <functional>
#include <map>
#include <atomic>

namespace uvxx
{
    typedef long long event_token;

    #define MULTICAST_EVENT_WITH_ARGS(event_name, owner_type, callback_args) \
    public: class event_##event_name : public uvxx::multicast_delegate_args<owner_type, callback_args>{}; \
    private:      event_##event_name _##event_name;\
    public:       event_##event_name &  ##event_name (){ return _##event_name;}\

    #define MULTICAST_EVENT(event_name, owner_type) \
    public: class event_##event_name : public uvxx::multicast_delegate<owner_type>{}; \
    private:      event_##event_name _##event_name ;\
    public:       event_##event_name & event_name (){ return _##event_name ;}\

    template<typename T0, typename...TARGS>
    class multicast_delegate_args
    {
    public:
        multicast_delegate_args() : m_tokenCounter(0)
        {

        }
       
        multicast_delegate_args(const multicast_delegate_args&) = default;
        multicast_delegate_args& operator=(const multicast_delegate_args&) = default;

        typedef std::function<void (T0*, TARGS...args)> callback_function;

    public:
        void invoke(T0* sender, TARGS...args )
        {
            for( auto i = m_handlerMap.begin(); i != m_handlerMap.end(); ++i )
            {
                (*i).second(sender, args...);
            }
        }

        void operator()(T0* sender, TARGS...args)
        {
            invoke( sender, args...);
        }

        event_token operator += ( callback_function f ) const
        {
            auto token = ++m_tokenCounter;

            m_handlerMap[token] = f;
            return token;
        }

        event_token operator -= ( event_token token ) const
        {
            m_handlerMap.erase(token);
            return token;
        }

    private:
        mutable std::atomic_uint_least64_t m_tokenCounter;
        mutable std::map<event_token, callback_function> m_handlerMap;
    };

    template<typename T0>
    class multicast_delegate
    {
    public:
        multicast_delegate() : m_tokenCounter(0)
        {

        }

        ~multicast_delegate()
        {

        }
        typedef std::function<void (T0*)> callback_function;

        multicast_delegate(const multicast_delegate& rhs)
        {
             m_tokenCounter = 0;//rhs.m_tokenCounter;
             m_handlerMap = rhs.m_handlerMap;
        }

        multicast_delegate& operator=(const multicast_delegate& rhs)
        {
            m_tokenCounter = rhs.m_tokenCounter;
            m_handlerMap = rhs.m_handlerMap;

            return *this;
        }

    public:
        void invoke(T0* sender)
        {
            if (m_handlerMap.empty())
            {
                return;
            }
            for( auto i = m_handlerMap.begin(); i != m_handlerMap.end(); ++i )
            {
                (*i).second(sender);
            }
        }

        void operator()(T0* sender)
        {
            invoke( sender);
        }

        event_token operator += ( callback_function f ) const
        {
            auto token = m_tokenCounter++;

            m_handlerMap[token] = f;
            return token;
        }

        event_token operator -= ( event_token token ) const
        {
            m_handlerMap.erase(token);
            return token;
        }

    private:
        mutable std::atomic_uint_least64_t m_tokenCounter;
        mutable std::map<event_token, callback_function> m_handlerMap;
    };
}
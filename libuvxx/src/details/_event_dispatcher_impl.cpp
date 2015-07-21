#include <thread>

#include "uvxx_exception.hpp"
#include "details/_event_dispatcher_impl.hpp"
#include "details/_event_dispatcher_frame_impl.hpp"

using namespace std;
using namespace uvxx::details;

#if defined(_MSC_VER) 
#define __thread_local __declspec(thread) 
#else
#define __thread_local thread_local
#endif

namespace uvxx { namespace details
{
    _event_dispatcher_impl_weak_ptr _last_dispatcher;

    __thread_local bool _has_initialized_thread_cache = false;

    struct _thread_info_cache
    {
        bool _has_dispatcher;
        std::thread::id _this_thread_id;

        void Initialize()
        {
            if (_has_initialized_thread_cache)
            {
                return;
            }

            _has_dispatcher = false;

            _this_thread_id = std::this_thread::get_id();

            _has_initialized_thread_cache = true;
        }

        bool has_dispatcher_get()
        {
            Initialize();
          
            return _has_dispatcher;
        }

        std::thread::id this_thread_id()
        {
            Initialize();

            return _this_thread_id;
        }

        void has_dispatcher_set(bool has_dispatcher)
        {
            Initialize();

            _has_dispatcher = has_dispatcher;
        }
    };

    __thread_local _thread_info_cache _thread_cache;

    _event_dispatcher_impl::_event_dispatcher_impl() : 
        _loop(false), 
        _threadId(_thread_cache.this_thread_id()), 
        _exitAllFrames(false), 
        _hasShutdownStarted(false), 
        _startingShutdown(false), 
        _hasShutdownFinished(false), 
        _hasShutdown(false),
        _frameDepth(0)
    {
        _thread_cache.has_dispatcher_set(true);
    }
    
    _event_dispatcher_impl::~_event_dispatcher_impl()
    {
        _thread_cache.has_dispatcher_set(false);
    }

    void _event_dispatcher_impl::push_frame_impl(event_dispatcher_frame_ptr const& frame)
    {
        _frameDepth++;

        while (frame->get_continue())
        {
            _loop.run_once();
        }

        if (_frameDepth == 1)
        {
            if (_hasShutdown)
            {
                start_shutdown_impl();
            }
        }

        _frameDepth--;

        if (_frameDepth == 0)
        {
            _exitAllFrames = false;
        }
    }

    void _event_dispatcher_impl::run()
    {
        _event_dispatcher_impl_ptr dispatcher = current_dispatcher();

        dispatcher->push_frame(make_shared<_event_dispatcher_frame_impl>());
    }

    void _event_dispatcher_impl::exit_all_frames()
    {
        _event_dispatcher_impl_ptr dispatcher = current_dispatcher();

        if (dispatcher->_frameDepth > 0)
        {
            dispatcher->_exitAllFrames = true;

            dispatcher->begin_invoke([]{});
        }
    }

    void _event_dispatcher_impl::push_frame(event_dispatcher_frame_ptr const& frame)
    {
        if (frame == nullptr)
        {
            throw uvxx_exception("frame cannot be null");
        }

        auto dispatcher = current_dispatcher();

        if (frame->dispatcher() != dispatcher)
        {
            throw uvxx_exception("cannot push frame belonging to another dispatcher");
        }

        dispatcher->push_frame_impl(frame);
    }

    _event_dispatcher_impl_ptr _event_dispatcher_impl::current_dispatcher()
    {
        thread::id this_id = this_thread::get_id();

        auto eventdispatcher = from_thread(this_id);

        if (eventdispatcher == nullptr)
        {
            struct event_dispatcher_make_shared_enabler : public _event_dispatcher_impl{};

            eventdispatcher = make_shared<event_dispatcher_make_shared_enabler>();

            std::lock_guard<std::mutex> lock(_global_lock());

            _dispatchers().push_back(eventdispatcher);
        }

        return eventdispatcher;
    }

    _event_dispatcher_impl_ptr _event_dispatcher_impl::from_thread(thread::id const & thread_id)
    {
        if (_dispatchers().empty())
        {
            return nullptr;
        }

        _event_dispatcher_impl_ptr returned_dispatcher;

        try
        {
            returned_dispatcher = _last_dispatcher.lock();
        }
        catch (std::bad_weak_ptr&)
        {

        }

        if (returned_dispatcher != nullptr && 
            returned_dispatcher->thread_id() == thread_id)
        {
            return returned_dispatcher;
        }
       
        returned_dispatcher = nullptr;
        
        std::lock_guard<std::mutex> lock(_global_lock());

        for (size_t i = 0; i < _dispatchers().size(); i++)
        {
            _event_dispatcher_impl_ptr dispatcher;

            try
            {
                dispatcher = _dispatchers()[i].lock();
            }
            catch (std::bad_weak_ptr&)
            {
                _dispatchers().erase(begin(_dispatchers()) + i);
                i--;
                continue;	
            }
                    
            if (dispatcher->thread_id() == thread_id)
            {
                returned_dispatcher = std::move(dispatcher);
            }
          
            if (returned_dispatcher != nullptr)
            {
                _last_dispatcher = returned_dispatcher;
            }
        }
        
        return returned_dispatcher;
    }

    bool _event_dispatcher_impl::check_access() const
    {
        thread::id this_id;

        /* static initialization issue with thread local cache hack */
        while (true)
        {
            this_id = _thread_cache.this_thread_id();

            if (this_id == std::thread::id())
            {
                _has_initialized_thread_cache = false;
                _thread_cache.Initialize();
                _thread_cache.has_dispatcher_set(true);
                continue;
            }

            break;
        }
        
        if (this_id == _threadId)
        {
            return true;
        }

        return false;
    }

    thread::id const & _event_dispatcher_impl::thread_id() const
    {
        assert(_threadId != std::thread::id());
        return _threadId;
    }

    void _event_dispatcher_impl::begin_shutdown()
    {
        begin_invoke([this]
        {
            start_shutdown_impl();
        });
    }

    void _event_dispatcher_impl::start_shutdown_impl()
    {
        if (!_startingShutdown)
        {
            _startingShutdown = true;

            _hasShutdownStarted = true;

            if (_frameDepth <= 0)
            {
                 shutdown_impl();
            }
            else
            {
                begin_invoke([]{});
            }
        }
    }

    void _event_dispatcher_impl::shutdown_impl()
    {
        if (!_hasShutdownFinished)
        {
            _hasShutdownFinished = true;
        }
    }

    void _event_dispatcher_impl::verify_access() const
    {
        if (check_access())
        {
            return;
        }

        throw uvxx_exception("cannot access from a foreign thread");
    }

    bool _event_dispatcher_impl::has_dispatcher()
    {
        return _thread_cache.has_dispatcher_get();
    }

    void _event_dispatcher_impl::begin_invoke_impl(std::function<void()> callback)
    {
        _loop.begin_invoke(std::move(callback));
    }

    void _event_dispatcher_impl::begin_invoke_impl_unsafe(std::function<void()> callback)
    {
        _loop.begin_invoke_unsafe(std::move(callback));
    }

    std::thread::id _event_dispatcher_impl::this_thread_id()
    {
        std::thread::id id = _thread_cache.this_thread_id();

        assert(id != std::thread::id());

        return _thread_cache.this_thread_id();
    }

    std::vector<_event_dispatcher_impl_weak_ptr>& _event_dispatcher_impl::_dispatchers()
    {
        static std::vector<_event_dispatcher_impl_weak_ptr>* dispatchers = new std::vector<_event_dispatcher_impl_weak_ptr>();

        return *dispatchers;
    }

    std::mutex& _event_dispatcher_impl::_global_lock()
    {
        static std::mutex* global_lock = new std::mutex();

        return *global_lock;
    }

}}
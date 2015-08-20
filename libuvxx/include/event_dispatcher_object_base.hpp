#pragma once

#include "event_dispatcher_object.hpp"

namespace uvxx
{
    template<typename INTERNAL_IMPL_T>
    class event_dispatcher_object_base : public event_dispatcher_object
    {
    public:
        event_dispatcher_object_base() :
            /* In case the last reference of our class gets cleared by an foreign thread
               we need to make sure it gets deleted on the correct dispatcher */
            _container(std::shared_ptr<_private_impl_container>(new _private_impl_container,[=](_private_impl_container * container)
        {
            if (container->__impl && !check_access())
            {
                dispatcher().begin_invoke([impl = std::move(container->__impl)]() mutable
                {
                    impl = nullptr;
                });
            }

            delete container;
        }))
        {
        }

        event_dispatcher_object_base(const event_dispatcher_object_base& rhs) = default;
      
        event_dispatcher_object_base& operator=(const event_dispatcher_object_base& rhs) = default;

        event_dispatcher_object_base(event_dispatcher_object_base&& rhs)
        {
            *this = std::move(rhs);
        }

        event_dispatcher_object_base& operator=(event_dispatcher_object_base&& rhs)
        {
            _container = std::move(rhs->_container);

            return *this;
        }

        bool operator=(std::nullptr_t rhs)
        {
            return _container->__impl = nullptr;
        }

        operator bool()
        {
            return (_container->__impl != nullptr);
        }

        bool operator==(std::nullptr_t rhs)
        {
            return (_container->__impl == nullptr);
        }

        bool operator !=(std::nullptr_t rhs)
        {
            return (_container->__impl != nullptr);
        }

        virtual ~event_dispatcher_object_base()
        {
           
        }

    protected:
        void private_impl_set(const std::shared_ptr<INTERNAL_IMPL_T>& impl)
        {
            _container->__impl = impl;
        }

        std::shared_ptr<INTERNAL_IMPL_T>& private_impl() const 
        {
            return _container->__impl;
        }

    private:
        struct _private_impl_container
        {
            std::shared_ptr<INTERNAL_IMPL_T> __impl;
        };

        std::shared_ptr<_private_impl_container> _container;
    };
}
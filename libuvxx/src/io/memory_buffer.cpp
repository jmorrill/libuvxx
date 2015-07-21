#include "io/memory_buffer.hpp"
#include <vector>

namespace uvxx { namespace io 
{
    class _memory_buffer_impl
    {
    private:
        std::vector<uint8_t> _buffer;
    public:
        _memory_buffer_impl(size_t initial_size) :
            _buffer(initial_size)
        {

        }

        ~_memory_buffer_impl() = default;

        _memory_buffer_impl(const _memory_buffer_impl&) = delete;

        _memory_buffer_impl& operator=(const _memory_buffer_impl&) = delete;

        uint8_t* data()
        {
            return _buffer.data();
        }

        void read(uint8_t* destination, size_t position, size_t size)
        {
            if (size > length_get() + position)
            {
                throw std::runtime_error("destination buffer size larger than buffer length");
            }

            if (destination == nullptr)
            {
                throw std::runtime_error("invalid destination pointer");
            }

            memcpy(destination, _buffer.data() + position, size);
        }

        void write(uint8_t* source, size_t position, size_t size)
        {
            if (size > length_get() + position)
            {
                throw std::runtime_error("destination buffer size larger than buffer length");
            }

            if (source == nullptr)
            {
                throw std::runtime_error("invalid source pointer");
            }

            memcpy(_buffer.data() + position, source, size);
        }

        void length_set(size_t size)
        {
            _buffer.resize(size);
        }

        size_t length_get() const
        {
            return _buffer.size();
        }

        void capacity_set(size_t size)
        {
            _buffer.reserve(size);
        }

        size_t capacity_get() const
        {
            return _buffer.capacity();
        }
    };

    memory_buffer::memory_buffer(size_t initial_size) : 
        __memory_buffer_impl(std::make_shared<_memory_buffer_impl>(initial_size))
    {

    }

    memory_buffer::memory_buffer(memory_buffer&& buffer)
    {
        *this = std::move(buffer);
    }

    memory_buffer::memory_buffer(const std::vector<uint8_t>& buffer) : memory_buffer(buffer.size())
    {
        memcpy(__memory_buffer_impl->data(), buffer.data(), buffer.size());
    }

    memory_buffer::memory_buffer(std::nullptr_t ptr)
    {
        __memory_buffer_impl = nullptr;
    }

    memory_buffer& memory_buffer::operator=(memory_buffer&& rhs)
    {
        __memory_buffer_impl = std::move(rhs.__memory_buffer_impl);

        return *this;
    }

    bool memory_buffer::operator=(std::nullptr_t rhs)
    {
        return __memory_buffer_impl == nullptr;
    }

    memory_buffer::~memory_buffer()
    {

    }

    void memory_buffer::length_set(size_t size)
    {
        __memory_buffer_impl->length_set(size);
    }

    size_t memory_buffer::length_get() const
    {
        return __memory_buffer_impl->length_get();
    }

    void memory_buffer::capacity_set(size_t size)
    {
        __memory_buffer_impl->capacity_set(size);
    }

    size_t memory_buffer::capacity_get() const
    {
        return __memory_buffer_impl->capacity_get();
    }

    void memory_buffer::read(uint8_t* destination, size_t position, size_t size) const
    {
        __memory_buffer_impl->read(destination, position, size);
    }

    void memory_buffer::write(uint8_t* source, size_t position, size_t size)
    {
        __memory_buffer_impl->write(source, position, size);
    }

    bool memory_buffer::operator==(std::nullptr_t rhs) const
    {
        return __memory_buffer_impl.get() != nullptr;
    }

    memory_buffer::operator bool() const
    {
        return __memory_buffer_impl.get() != nullptr;
    }

    memory_buffer::operator uint8_t*() const
    {
        return __memory_buffer_impl->data();
    }

    void* memory_buffer::operator[](size_t index) const
    {
        return __memory_buffer_impl->data() + index;
    }

    memory_buffer::operator char*() const
    {
        return reinterpret_cast<char*>(__memory_buffer_impl->data());
    }

    uint8_t* memory_buffer::data() const
    {
        return reinterpret_cast<uint8_t*>(__memory_buffer_impl->data());
    }

}}
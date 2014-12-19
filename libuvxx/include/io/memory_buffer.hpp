#pragma once
#include <memory>

namespace uvxx { namespace io 
{
    class _memory_buffer_impl;

    class memory_buffer
    {
    public:
        explicit memory_buffer(size_t initial_size = 0);

        ~memory_buffer();

        memory_buffer(const memory_buffer&) = default;

        memory_buffer& operator=(const memory_buffer&) = default;

        memory_buffer(memory_buffer&& dispatcher);

        memory_buffer& operator=(memory_buffer&& rhs);

        bool operator==(std::nullptr_t rhs) const;

        bool operator=(std::nullptr_t rhs);

        operator uint8_t*() const;

        operator char*() const;

        operator bool() const;

        void length_set(size_t size);

        size_t length_get() const;

        void capacity_set(size_t size);

        size_t capacity_get() const;

        void read(uint8_t* destination, size_t position, size_t size) const;

        void write(uint8_t* source, size_t position, size_t size);

    private:
        std::shared_ptr<_memory_buffer_impl> __memory_buffer_impl;
    };
}}
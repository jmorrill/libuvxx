#pragma once

namespace uvxx { namespace details { namespace ltalloc 
{
void*  ltalloc(size_t);
void   ltfree(void*);
size_t ltalloc_usable_size(void*);
void   ltalloc_squeeze(size_t pad);//return memory to the system (see http://code.google.com/p/ltalloc/wiki/Main#rmem for more details)

#ifdef max
#undef max
#endif
template<typename T>
class ltalloc_allocator
{
public:
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef T value_type;


    ltalloc_allocator() throw() {};
    ltalloc_allocator(const ltalloc_allocator& /*other*/) throw() {};

    template<typename U>
    ltalloc_allocator(const ltalloc_allocator<U>& /*other*/) throw() {};

    template<typename U>
    ltalloc_allocator& operator = (const ltalloc_allocator<U>& /*other*/) { return *this; }
    ltalloc_allocator<T>& operator = (const ltalloc_allocator& /*other*/) { return *this; }
    ~ltalloc_allocator() {}

    pointer address(reference value) const { return &value; }
    const_pointer address(const_reference value) const { return &value; }

    pointer allocate(size_type n, const void* /*hint*/ = 0) { return static_cast<pointer> (ltalloc(n * sizeof(value_type))); }
    void deallocate(void* ptr, size_type /*n*/) { ltfree(static_cast<T*> (ptr)); }

    //template<typename U, typename... Args>
    //void construct(U* ptr, Args&&  ... args) { ::new (static_cast<void*> (ptr)) U(std::forward<Args>(args)...); }
    //void construct(pointer ptr, const T& val) { new (static_cast<T*> (ptr)) T(val); }

    template<typename U>
    void destroy(U* ptr) { ptr->~U(); }
    void destroy(pointer ptr) { ptr->~T(); }

    size_type max_size() const { return std::numeric_limits<std::size_t>::max() / sizeof(T); } /**return std::size_t(-1);**/

    template<typename U>
    struct rebind
    {
        typedef ltalloc_allocator<U> other;
    };
};
}}}

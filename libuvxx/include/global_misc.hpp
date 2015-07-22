#pragma once

// Define operator overloads to enable bit operations on enum values that are 
// used to define flags. Use DEFINE_ENUM_FLAG_OPERATORS(YOUR_TYPE) to enable these 
// operators on YOUR_TYPE.

// Moved here from objbase.w.

// Templates are defined here in order to avoid a dependency on C++ <type_traits> header file,
// or on compiler-specific contructs.
extern "C++" {

    template <size_t S>
    struct __ENUM_FLAG_INTEGER_FOR_SIZE;

    template <>
    struct __ENUM_FLAG_INTEGER_FOR_SIZE<1>
    {
        typedef signed char type;
    };

    template <>
    struct __ENUM_FLAG_INTEGER_FOR_SIZE<2>
    {
        typedef signed short  type;
    };

    template <>
    struct __ENUM_FLAG_INTEGER_FOR_SIZE<4>
    {
        typedef signed int type;
    };

    // used as an approximation of std::underlying_type<T>
    template <class T>
    struct __ENUM_FLAG_SIZED_INTEGER
    {
        typedef typename __ENUM_FLAG_INTEGER_FOR_SIZE<sizeof(T)>::type type;
    };
}

#define DEFINE_ENUM_FLAG(ENUMTYPE) extern "C++" { \
inline ENUMTYPE operator | (ENUMTYPE a, ENUMTYPE b) { return ENUMTYPE(((__ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)a) | ((__ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)b)); } \
inline ENUMTYPE &operator |= (ENUMTYPE &a, ENUMTYPE b) { return (ENUMTYPE &)(((__ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type &)a) |= ((__ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)b)); } \
inline ENUMTYPE operator & (ENUMTYPE a, ENUMTYPE b) { return ENUMTYPE(((__ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)a) & ((__ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)b)); } \
inline ENUMTYPE &operator &= (ENUMTYPE &a, ENUMTYPE b) { return (ENUMTYPE &)(((__ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type &)a) &= ((__ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)b)); } \
inline ENUMTYPE operator ~ (ENUMTYPE a) { return ENUMTYPE(~((__ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)a)); } \
inline ENUMTYPE operator ^ (ENUMTYPE a, ENUMTYPE b) { return ENUMTYPE(((__ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)a) ^ ((__ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)b)); } \
inline ENUMTYPE &operator ^= (ENUMTYPE &a, ENUMTYPE b) { return (ENUMTYPE &)(((__ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type &)a) ^= ((__ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)b)); } \
}

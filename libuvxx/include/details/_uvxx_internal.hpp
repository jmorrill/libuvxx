#pragma once
#include "uv.h"
#if !defined(_MSC_VER) 
#undef __cplusplus
#define __cplusplus 201103L
#endif

#if defined(_MSC_VER) 
#define _noexcept
#else
#define _noexcept noexcept
#endif

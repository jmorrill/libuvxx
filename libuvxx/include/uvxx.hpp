#pragma once

//#if defined(__GNUC__) 
//#undef __cplusplus
//#define __cplusplus 201103L
//#endif
#define LTALLOC_DISABLE_OPERATOR_NEW_OVERRIDE

#include "uvxx_exception.hpp"
#include "pplx/pplxtasks.h"
#include "pplx/pplpp.hpp"

#include "event_dispatcher.hpp"
#include "event_dispatcher_object.hpp"
#include "event_dispatcher_frame.hpp"
#include "event_dispatcher_timer.hpp"

#include "net/dns.hpp"
#include "net/stream_socket.hpp"

#include "io/memory_buffer.hpp"

#include "fs/path.hpp"
#include "fs/file.hpp"
#include "fs/directory.hpp"


#pragma once
#include <string>
#include "event_dispatcher_object.hpp"
#include "pplx/pplxtasks.h"

namespace uvxx { namespace net
{
    class dns
    {
        dns() = delete;

        ~dns();
    public:

        dns(const dns&) = delete;

        dns& operator=(const dns&) = delete;

        static uvxx::pplx::task<std::string> resolve_host_async(std::string const& host_name);
    };

   
}}
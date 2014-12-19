#pragma once
#include "_uvxx_internal.hpp"

namespace uvxx { namespace details 
{
    using ip4_addr = sockaddr_in;

    using ip6_addr = sockaddr_in6;

    ip4_addr to_ip4_addr(const std::string& ip, int port);

    ip6_addr to_ip6_addr(const std::string& ip, int port);

    bool from_ip4_addr(ip4_addr* src, std::string& ip, int& port);

    bool from_ip6_addr(ip6_addr* src, std::string& ip, int& port);
}}

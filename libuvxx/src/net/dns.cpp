#include <functional>
#include "details/_event_dispatcher_impl.hpp"
#include "net/details/_dns_impl.hpp"
#include "net/dns.hpp"

using namespace uvxx;
using namespace uvxx::pplx;
using namespace uvxx::details;
using namespace uvxx::net::details;

namespace uvxx { namespace net
{
    dns::~dns()
    {
    }

    task<std::string> dns::resolve_host_async(std::string const& host_name)
    {
       auto _dns = std::make_shared<_dns_impl>();

       return _dns->resolve_host_async(host_name).then([_dns](task<std::string> result)
       {
           return result;
       });
    }
}}
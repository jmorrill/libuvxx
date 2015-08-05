#include "details/_live_environment_base.hpp"

using namespace uvxx::rtsp::details;

_live_environment_base::_live_environment_base(): 
    _usage_environement(_get_live_environment())
{
}

_usage_environment_ptr _live_environment_base::live_usage_environment()
{
    return _usage_environement;
}
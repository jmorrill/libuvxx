#include "details/_rtsp_client_impl.hpp"

using namespace uvxx::uvxx_rtsp::details;

_rtsp_client_impl::_rtsp_client_impl()
{
    _task_scheduler = std::unique_ptr<_uvxx_task_scheduler>(_uvxx_task_scheduler::createNew());

    _usage_environment = BasicUsageEnvironment::createNew(*(_task_scheduler.get()));
}

uvxx::pplx::task<void> uvxx::uvxx_rtsp::details::_rtsp_client_impl::open(const std::string& url)
{
    _live_client = std::make_unique<_live_rtsp_client>(*_usage_environment, url.c_str(), nullptr, 0);
    return uvxx::pplx::create_task([]{});
}


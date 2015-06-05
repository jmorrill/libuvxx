#include "event_dispatcher_object.hpp"

namespace uvxx { namespace uvxx_rtsp { namespace details
{
    /* implementation forward */
    class _rtsp_client_impl;
}}}

namespace uvxx { namespace uvxx_rtsp 
{
    class rtsp_client : public uvxx::event_dispatcher_object
    {
    public:
        rtsp_client();

        rtsp_client(const rtsp_client&) = default;

        rtsp_client& operator=(const rtsp_client&) = default;

        rtsp_client(rtsp_client&& dispatcher);

        rtsp_client& operator=(rtsp_client&& rhs);

    public:
        uvxx::pplx::task<void> open(const std::string& url);

    private:
        std::shared_ptr<details::_rtsp_client_impl> __rtsp_client_imp;
    };
}}
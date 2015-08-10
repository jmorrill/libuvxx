#include "details/_uv_loop.hpp"
#include "details/_uv_work.hpp"

using namespace std;

namespace uvxx { namespace details
{
    _uv_work::_uv_work(_uv_loop* uv_loop, std::function<void()> work_function) : 
        _work_function(std::move(work_function))
    {
        uv_loop_t* l = *(uv_loop);
        
        uv_queue_work(l, _handle, work_callback, work_callback_after);
    }

    void _uv_work::work_callback(uv_work_t* handle)
    {
        auto w = reinterpret_cast<_uv_work*>(handle->data);

                                    w->_work_function();
    }

    void _uv_work::work_callback_after(uv_work_t* handle, int /*status*/)
    {
        auto w = reinterpret_cast<_uv_work*>(handle->data);

        delete handle;

        delete w;
    }

    _uv_work::~_uv_work()
    {
    }
}}
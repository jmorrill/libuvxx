#include "stdafx.h"
#include "rtsp_client.hpp"
#include "streaming_media_session.hpp"
#include "rtsp_exceptions.hpp"

using namespace std;
using namespace uvxx;
using namespace uvxx::rtsp;
using namespace uvxx::pplx;

bool on_frame_callback()
{
    return true;
}

streaming_media_session stream;

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        return -1;
    }

    {
        uvxx::rtsp::rtsp_client client;

        client.open(argv[1]).then([=]
        {
            auto id = std::this_thread::get_id();

            return client.play(); 
        }).then([&](task<streaming_media_session> t)
        {
            stream = std::move(t.get());
            
            stream.on_frame_callback_set(on_frame_callback);
        }).then([client]
        {
            return create_timer_task(std::chrono::milliseconds(35000));
        }).then([](task<void> t)
        {
            try
            {
                t.get();
            }
            catch (const rtsp_exception& e)
            {
                printf(e.what());
            }

            event_dispatcher::current_dispatcher().begin_shutdown();
        });

        event_dispatcher::run();
    }


    return 0;
}

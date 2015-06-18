#include "stdafx.h"
#include "rtsp_client.hpp"
#include "streaming_media_session.hpp"
#include "rtsp_exceptions.hpp"
#include "media_sample.hpp"

using namespace std;
using namespace uvxx;
using namespace uvxx::rtsp;
using namespace uvxx::pplx;

bool on_frame_callback(const media_sample& sample)
{
    printf("c: %s\t size: %d\t truncated: %d\t time: %llu \t s:%u cs:%u\n",
        sample.codec_name().c_str(),
        sample.size(),
        sample.is_truncated(),
        sample.presentation_time(),
        sample.stream_number(),
        sample.is_complete_sample());

    return true;
}


int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        return -1;
    }

    streaming_media_session stream;

    {
        uvxx::rtsp::rtsp_client client;
        client.username_set("admin");
        client.password_set("12345");
        client.protocol_set(transport_protocol::tcp);
 
        client.open(argv[1]).then([=]
        {
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

#include "stdafx.h"
#include "rtsp_client.hpp"
#include "streaming_media_session.hpp"
#include "rtsp_exceptions.hpp"
#include "media_sample.hpp"

using namespace std;
using namespace uvxx;
using namespace uvxx::pplx;
using namespace uvxx::rtsp;
using namespace uvxx::rtsp::media_sample_attributes;

bool on_frame_callback(const media_sample& sample)
{
    printf("codec: %s\t size: %d\t pts: %llu \t s:%u",
           sample.codec_name().c_str(),
           sample.size(),
           sample.presentation_time(),
           sample.stream_number());

    if (sample.codec_name() == "H264")
    {
        auto video_size = sample.attribute_get<video_dimensions>(ATTRIBUTE_VIDEO_DIMENSIONS);

        bool key_frame = sample.attribute_get<bool>(ATTRIBUTE_VIDEO_KEYFRAME);

        printf("\twxh: %dx%d", video_size.width, video_size.height);

        if (key_frame)
        {
            printf("\tkey_frame");
        }
    }

    printf("\n");

    return true;
}


int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        return -1;
    }

    {
        uvxx::rtsp::rtsp_client client;

        client.username_set("admin");
        client.password_set("12345");
        client.protocol_set(transport_protocol::udp);
 
        client.open(argv[1]).then([=]
        {
            return client.play(); 
        }).then([client]
        {
            client.begin_stream_read(on_frame_callback);
        }).then([client]
        {
            return create_timer_task(std::chrono::milliseconds(45000));
        }).then([](task<void> t)
        {
            try
            {
                t.get();
            }
            catch (const rtsp_exception& e)
            {
                printf("exception: ");
                printf(e.what());
            }

            event_dispatcher::current_dispatcher().begin_shutdown();
        });

        event_dispatcher::run();
    }


    return 0;
}

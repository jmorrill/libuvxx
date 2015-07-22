#include "stdafx.h"
#include "rtsp_client.hpp"

#include "rtsp_exceptions.hpp"
#include "media_sample.hpp"

using namespace std;
using namespace uvxx;
using namespace uvxx::pplx;
using namespace uvxx::rtsp;
using namespace uvxx::rtsp::sample_attributes;

rtsp_client client;

void on_sample_callback(const media_sample& sample)
{
    printf("codec: %s\t size: %d\t pts: %lld \t s:%u",
           sample.codec_name().c_str(),
           sample.size(),
           sample.presentation_time().count(),
           sample.stream_number());

    if (sample.attribute_get<sample_major_type>(ATTRIBUTE_SAMPLE_MAJOR_TYPE) == sample_major_type::video)
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

	client.read_stream_sample();
}


int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        return -1;
    }

    {
        //client = rtsp_client();

        client.on_sample_set(on_sample_callback);

        client.credentials_set("admin", "12345");

        client.protocol_set(transport_protocol::tcp);
 
        client.open(argv[1]).then([=]
        {
            return client.play(); 
        }).then([]
        {
            client.read_stream_sample();
        }).then([]
        {
            return create_timer_task(std::chrono::milliseconds(45000));
        }).then([](task<void> t)
        {
            try
            {
                t.get();
            }
            catch (const rtsp_network_timeout& /*e*/)
            {
                printf("timeout\n");
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

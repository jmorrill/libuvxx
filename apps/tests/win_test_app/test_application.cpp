#include "stdafx.h"
#include "rtsp_client.hpp"

#include "rtsp_exceptions.hpp"
#include "media_sample.hpp"
#include "sample_attributes.hpp"

#include "rtsp_server.hpp"    

using namespace std;
using namespace uvxx;
using namespace uvxx::pplx;
using namespace uvxx::rtsp;
using namespace uvxx::rtsp::sample_attributes;

std::unique_ptr<rtsp_client> client;

void on_sample_callback(const media_sample& sample)
{
    create_task([=]
    {
        auto stats = client->stream_statistics_get(sample.stream_number());

        if (stats.percent_packet_loss)
        {
            printf("packet loss: %5.2f%%\n", stats.percent_packet_loss);
        }

        printf("codec: %s\t size: %d\t pts: %lld s:%u",
            sample.codec_name().c_str(),
            sample.size(),
            sample.presentation_time().count(),
            sample.stream_number());

        auto major_type = sample.attribute_get<sample_major_type>(ATTRIBUTE_SAMPLE_MAJOR_TYPE);

        if (major_type == sample_major_type::video)
        {
            auto video_size = sample.attribute_get<video_dimensions>(ATTRIBUTE_VIDEO_DIMENSIONS);

            bool key_frame = sample.attribute_get<bool>(ATTRIBUTE_VIDEO_KEYFRAME);

            printf("\twxh: %dx%d", video_size.width, video_size.height);

            if (key_frame)
            {
                printf("\tkey_frame");
            }
        }
        else if (major_type == sample_major_type::audio)
        {
            auto samples_per_second = sample.attribute_get<int>(ATTRIBUTE_AUDIO_SAMPLES_PER_SECOND);

            auto channels = sample.attribute_get<int>(ATTRIBUTE_AUDIO_CHANNEL_COUNT);

            printf("\tfreq: %d", samples_per_second);

            printf("\tchannels: %d", channels);
        }

        printf("\n");
    }).then([=]
    {
        client->read_stream_sample();
    });
}

void stream_closed(int stream_number)
{
    printf("%d stream closed\n", stream_number);
}


int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        return -1;
    }

   /* rtsp_server server;

    server.start_server(8554);*/

    //event_dispatcher::run();

    //return 0;

    printf("argv[1] is %s\n", argv[1]);

    client = std::make_unique<rtsp_client>();

    {
        client->on_sample_set(on_sample_callback);

        client->on_stream_closed_set(stream_closed);

        client->credentials_set("admin", "12345");

        client->protocol_set(transport_protocol::udp);

        client->open(argv[1]).then([=]
        {
            printf("starting play \n");
            return client->play();
        }).then([]
        {
            client->read_stream_sample();
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
            catch (const std::exception& e)
            {
                printf("exception: ");
                printf(e.what());
            }

            event_dispatcher::current_dispatcher().begin_shutdown();
        });

        event_dispatcher::run();
    }
    client = nullptr;
    return 0;
}
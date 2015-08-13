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

rtsp_client client;
server_media_session server_session;

void on_sample_callback(const media_sample& sample)
{
    create_task([=]
    {
        auto stats = client.stream_statistics_get(sample.stream_number());

        if (stats.percent_packet_loss)
        {
            //printf("packet loss: %5.2f%%\n", stats.percent_packet_loss);
        }

        /* printf("codec: %s\t size: %d\t pts: %lld s:%u",
            sample.codec_name().c_str(),
            sample.size(),
            sample.presentation_time().count(),
            sample.stream_number());*/

        auto major_type = sample.attribute_get<sample_major_type>(ATTRIBUTE_SAMPLE_MAJOR_TYPE);

        if (major_type == sample_major_type::video)
        {
            bool key_frame = sample.attribute_get<bool>(ATTRIBUTE_VIDEO_KEYFRAME);

            if(server_session)
            {
                server_session.deliver_sample(1, sample);
            }

            auto video_size = sample.attribute_get<video_dimensions>(ATTRIBUTE_VIDEO_DIMENSIONS);

           // printf("\twxh: %dx%d", video_size.width, video_size.height);

            if (key_frame)
            {
             //   printf("\tkey_frame");
            }
        }
        else if (major_type == sample_major_type::audio)
        {
            auto samples_per_second = sample.attribute_get<int>(ATTRIBUTE_AUDIO_SAMPLES_PER_SECOND);

            auto channels = sample.attribute_get<int>(ATTRIBUTE_AUDIO_CHANNEL_COUNT);

            //printf("\tfreq: %d", samples_per_second);

            //printf("\tchannels: %d", channels);
        }

        //printf("\n");
    }).then([=]
    {
        client.read_stream_sample();
    });
}

void stream_closed(int stream_number)
{
    printf("%d stream closed\n", stream_number);
}


task<server_media_session> on_session_requested(const std::string& stream_name)
{
    printf("creating session\n");

    server_session = server_media_session();

    media_descriptor descriptor;

    descriptor.add_stream_from_attributes(1, "H264", media_attributes());

    server_session.set_media_descriptor(descriptor);

    return task_from_result(server_session);
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        return -1;
    }

    rtsp_server server;

    server.start_server(8554);

    server.on_session_request_set(on_session_requested);

    printf("argv[1] is %s\n", argv[1]);

    {
        client.on_sample_set(on_sample_callback);

        client.on_stream_closed_set(stream_closed);

        client.credentials_set("admin", "12345");

        client.protocol_set(transport_protocol::udp);

        client.open(argv[1]).then([=]
        {
            printf("starting play \n");
            return client.play();
        }).then([]
        {
            client.read_stream_sample();
        }).then([]
        {
            return create_timer_task(std::chrono::milliseconds(55000));
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

            //event_dispatcher::current_dispatcher().begin_shutdown();
        });

        event_dispatcher::run();
    }

    return 0;
}
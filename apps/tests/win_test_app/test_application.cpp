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
std::vector<server_media_session> server_sessions;

void on_sample_callback(const media_sample& sample)
{
    create_task([=]
    {
        auto stats = client.stream_statistics_get(sample.stream_number());

        if (stats.percent_packet_loss)
        {
            //printf("packet loss: %5.2f%%\n", stats.percent_packet_loss);
        }

        auto major_type = sample.attribute_get<sample_major_type>(ATTRIBUTE_SAMPLE_MAJOR_TYPE);

        if (major_type == sample_major_type::video)
        {
            for(auto& session : server_sessions)
            {
                session.deliver_sample(sample.stream_number(), sample);
            }
        }
        else if (major_type == sample_major_type::audio)
        {
            for (auto& session : server_sessions)
            {
                session.deliver_sample(sample.stream_number(), sample);
            }
        }

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

    auto server_session = server_media_session();

    try
    {
        media_descriptor descriptor = client.media_descriptor_get();

        server_session.set_media_descriptor(descriptor);
    }
    catch (const std::exception&e)
    {
        return task_from_result(server_session);
    }

    server_sessions.push_back(server_session);

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

        client.protocol_set(transport_protocol::tcp);

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
            catch (const std::exception& turn_down_for)
            {
                printf(turn_down_for.what());
            }

            //event_dispatcher::current_dispatcher().begin_shutdown();
        });

        event_dispatcher::run();
    }

    return 0;
}
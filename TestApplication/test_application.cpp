#include "stdafx.h"
#include "rtsp_client.hpp"
#include "streaming_media_session.hpp"

using namespace std;
using namespace uvxx;
using namespace uvxx::rtsp;
using namespace uvxx::pplx;



bool on_frame_callback()
{
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

        client.open(argv[1]).then([=]
        {
            return client.play(); 
        }).then([&](task<uvxx::rtsp::streaming_media_session> t) mutable
        {
            stream = std::move(t.get());
            
            stream.on_frame_callback_set(on_frame_callback);
        });

        event_dispatcher::run();
    }


    return 0;
}

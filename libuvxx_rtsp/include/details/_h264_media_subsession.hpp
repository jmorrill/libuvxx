#pragma once
#include "event_dispatcher_object.hpp"
#include "event_dispatcher_frame.hpp"
#include "_live_server_media_subsession.hpp"

namespace uvxx { namespace rtsp { namespace details 
{
    class _live_framed_source;

    class _h264_framed_source;

    using framed_source_created_delegate = std::function<void(int stream_id, unsigned client_session_id, const std::shared_ptr<_live_framed_source>& source)>;

    class _h264_media_subsession : public _live_server_media_subsession
    {
    public:
        explicit _h264_media_subsession(int stream_id);

        _h264_media_subsession(const _h264_media_subsession&) = delete;

        const _h264_media_subsession& operator=(const _h264_media_subsession&) = delete;

        virtual ~_h264_media_subsession();

    protected:
        virtual FramedSource* createNewStreamSource(unsigned client_session_id, unsigned& estimated_kbps) override;

        virtual RTPSink* createNewRTPSink(Groupsock* rtp_groupsock, unsigned char rtp_payload_type_if_dynamic, FramedSource* input_source) override;

        virtual void closeStreamSource(FramedSource *inputSource) override;

        static void after_playing_dummy_callback(void* client_data);
       
    private:
        framed_source_created_delegate _framed_source_created;

    };
 }}}
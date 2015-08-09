#pragma once
#include "OnDemandServerMediaSubsession.hh"

#include "event_dispatcher_object.hpp"
#include "event_dispatcher_frame.hpp"

namespace uvxx { namespace rtsp { namespace details 
{
    class _live_framed_source;

    class _h264_framed_source;

    using framed_source_created_delegate = std::function<void(int stream_id, unsigned client_session_id, const std::shared_ptr<_live_framed_source>& source)>;

    class _h264_media_subsession : public OnDemandServerMediaSubsession, public event_dispatcher_object
    {
    public:
        explicit _h264_media_subsession(int stream_id);

        _h264_media_subsession(const _h264_media_subsession&) = delete;

        const _h264_media_subsession& operator=(const _h264_media_subsession&) = delete;

        virtual ~_h264_media_subsession();

        void framed_source_created_set(framed_source_created_delegate callback);

    protected:
        virtual char const* getAuxSDPLine(RTPSink* rtp_sink, FramedSource* input_source) override;

        virtual FramedSource* createNewStreamSource(unsigned client_session_id, unsigned& estimated_kbps) override;

        virtual RTPSink* createNewRTPSink(Groupsock* rtp_groupsock, unsigned char rtp_payload_type_if_dynamic, FramedSource* input_source) override;

        virtual void closeStreamSource(FramedSource *inputSource) override;

        void after_playing_dummy();

        static void after_playing_dummy_callback(void* client_data);
       
        bool check_for_aux_sdp_line();

    private:
        framed_source_created_delegate _framed_source_created;

        std::shared_ptr<_h264_framed_source> _source;

        uvxx::event_dispatcher_frame _sdp_check_dispatcher_frame;

        std::string _aux_sdp_line;

        RTPSink* fDummyRTPSink;

        int _stream_id;
    };
 }}}
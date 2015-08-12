#pragma once
#include "OnDemandServerMediaSubsession.hh"

#include "event_dispatcher_object.hpp"
#include "event_dispatcher_frame.hpp"

namespace uvxx { namespace rtsp { namespace details 
{
    class _live_framed_source;

    using framed_source_created_delegate = std::function<void(int stream_id, unsigned client_session_id, const std::shared_ptr<_live_framed_source>& source)>;

    class _live_server_media_subsession : public OnDemandServerMediaSubsession, public event_dispatcher_object
    {
    public:
        explicit _live_server_media_subsession(int stream_id);

        _live_server_media_subsession(const _live_server_media_subsession&) = delete;

        const _live_server_media_subsession& operator=(const _live_server_media_subsession&) = delete;

        virtual ~_live_server_media_subsession();

        void framed_source_created_set(framed_source_created_delegate callback);

        uvxx::pplx::task<void> preload_sdp_line();

    public:
        int stream_id();

    protected:

        void notify_framed_source(unsigned client_session_id);

    protected:
        virtual char const* getAuxSDPLine(RTPSink* rtp_sink, FramedSource* input_source) override;

        virtual FramedSource* createNewStreamSource(unsigned client_session_id, unsigned& estimated_kbps) override = 0;

        virtual RTPSink* createNewRTPSink(Groupsock* rtp_groupsock, unsigned char rtp_payload_type_if_dynamic, FramedSource* input_source) override = 0;

        virtual void closeStreamSource(FramedSource *inputSource) override;

        void after_playing_dummy();

        static void after_playing_dummy_callback(void* client_data);
       
        bool check_for_aux_sdp_line();

        std::shared_ptr<_live_framed_source> _source;

    private:
        framed_source_created_delegate _framed_source_created;

        uvxx::event_dispatcher_frame _sdp_check_dispatcher_frame;

        std::string _aux_sdp_line;

        RTPSink* _dummy_rtp_sink;

        int _stream_id;
    };

}}}
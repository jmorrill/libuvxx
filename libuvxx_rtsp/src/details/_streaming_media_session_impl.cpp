#include "details/_streaming_media_session_impl.hpp"
#include "details/_media_session_impl.hpp"
#include "MediaSession.hh"

using namespace uvxx::pplx;
using namespace uvxx::rtsp;
using namespace uvxx::rtsp::details;


const unsigned char * m_pStart;
unsigned short m_nLength;
int m_nCurrentBit;

unsigned int ReadBit()
{
    assert(m_nCurrentBit <= m_nLength * 8);
    int nIndex = m_nCurrentBit / 8;
    int nOffset = m_nCurrentBit % 8 + 1;

    m_nCurrentBit++;
    return (m_pStart[nIndex] >> (8 - nOffset)) & 0x01;
}

unsigned int ReadBits(int n)
{
    int r = 0;
    int i;
    for (i = 0; i < n; i++)
    {
        r |= (ReadBit() << (n - i - 1));
    }
    return r;
}

unsigned int ReadExponentialGolombCode()
{
    int r = 0;
    int i = 0;

    while ((ReadBit() == 0) && (i < 32))
    {
        i++;
    }

    r = ReadBits(i);
    r += (1 << i) - 1;
    return r;
}

unsigned int ReadSE()
{
    int r = ReadExponentialGolombCode();
    if (r & 0x01)
    {
        r = (r + 1) / 2;
    }
    else
    {
        r = -(r / 2);
    }
    return r;
}

void Parse(const unsigned char * pStart, unsigned short nLen)
{
    m_pStart = pStart;
    m_nLength = nLen;
    m_nCurrentBit = 0;

    int frame_crop_left_offset = 0;
    int frame_crop_right_offset = 0;
    int frame_crop_top_offset = 0;
    int frame_crop_bottom_offset = 0;

    int forbidden = ReadBit();
    int nal_ref_idc = ReadBits(2);
    int nal_unit_type = ReadBits(5);

    int profile_idc = ReadBits(8);

    if (nal_unit_type != 7)
    {
        return;
    }
    int constraint_set0_flag = ReadBit();
    int constraint_set1_flag = ReadBit();
    int constraint_set2_flag = ReadBit();
    int constraint_set3_flag = ReadBit();
    int constraint_set4_flag = ReadBit();
    int constraint_set5_flag = ReadBit();
    int reserved_zero_2bits = ReadBits(2);
    int level_idc = ReadBits(8);
    int seq_parameter_set_id = ReadExponentialGolombCode();


    if (profile_idc == 100 || profile_idc == 110 ||
        profile_idc == 122 || profile_idc == 244 ||
        profile_idc == 44 || profile_idc == 83 ||
        profile_idc == 86 || profile_idc == 118)
    {
        int chroma_format_idc = ReadExponentialGolombCode();

        if (chroma_format_idc == 3)
        {
            int residual_colour_transform_flag = ReadBit();
        }
        int bit_depth_luma_minus8 = ReadExponentialGolombCode();
        int bit_depth_chroma_minus8 = ReadExponentialGolombCode();
        int qpprime_y_zero_transform_bypass_flag = ReadBit();
        int seq_scaling_matrix_present_flag = ReadBit();

        if (seq_scaling_matrix_present_flag)
        {
            int i = 0;
            for (i = 0; i < 8; i++)
            {
                int seq_scaling_list_present_flag = ReadBit();
                if (seq_scaling_list_present_flag)
                {
                    int sizeOfScalingList = (i < 6) ? 16 : 64;
                    int lastScale = 8;
                    int nextScale = 8;
                    int j = 0;
                    for (j = 0; j < sizeOfScalingList; j++)
                    {
                        if (nextScale != 0)
                        {
                            int delta_scale = ReadSE();
                            nextScale = (lastScale + delta_scale + 256) % 256;
                        }
                        lastScale = (nextScale == 0) ? lastScale : nextScale;
                    }
                }
            }
        }
    }

    int log2_max_frame_num_minus4 = ReadExponentialGolombCode();
    int pic_order_cnt_type = ReadExponentialGolombCode();
    if (pic_order_cnt_type == 0)
    {
        int log2_max_pic_order_cnt_lsb_minus4 = ReadExponentialGolombCode();
    }
    else if (pic_order_cnt_type == 1)
    {
        int delta_pic_order_always_zero_flag = ReadBit();
        int offset_for_non_ref_pic = ReadSE();
        int offset_for_top_to_bottom_field = ReadSE();
        int num_ref_frames_in_pic_order_cnt_cycle = ReadExponentialGolombCode();
        int i;
        for (i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++)
        {
            ReadSE();
            //sps->offset_for_ref_frame[ i ] = ReadSE();
        }
    }
    int max_num_ref_frames = ReadExponentialGolombCode();
    int gaps_in_frame_num_value_allowed_flag = ReadBit();
    int pic_width_in_mbs_minus1 = ReadExponentialGolombCode();
    int pic_height_in_map_units_minus1 = ReadExponentialGolombCode();
    int frame_mbs_only_flag = ReadBit();
    if (!frame_mbs_only_flag)
    {
        int mb_adaptive_frame_field_flag = ReadBit();
    }
    int direct_8x8_inference_flag = ReadBit();
    int frame_cropping_flag = ReadBit();
    if (frame_cropping_flag)
    {
        frame_crop_left_offset = ReadExponentialGolombCode();
        frame_crop_right_offset = ReadExponentialGolombCode();
        frame_crop_top_offset = ReadExponentialGolombCode();
        frame_crop_bottom_offset = ReadExponentialGolombCode();
    }
    int vui_parameters_present_flag = ReadBit();
    pStart++;

    int Width = ((pic_width_in_mbs_minus1 + 1) * 16) - frame_crop_right_offset * 2 - frame_crop_left_offset * 2;
    int Height = ((2 - frame_mbs_only_flag)* (pic_height_in_map_units_minus1 + 1) * 16) - (frame_crop_top_offset * 2) - (frame_crop_bottom_offset * 2);

    printf("\n\nWxH = %dx%d\n\n", Width, Height);

}

static bool isH264iFrame(byte* paket)
{
    int RTPHeaderBytes = 0;

    int fragment_type = paket[RTPHeaderBytes + 0] & 0x1F;
    int nal_type = paket[RTPHeaderBytes + 1] & 0x1F;
    int start_bit = paket[RTPHeaderBytes + 1] & 0x80;

    if (fragment_type == 5)
    {
        return true;
    }

    return false;
}



struct _streaming_media_io_state
{
    _streaming_media_io_state() = default;

    _streaming_media_io_state(const _streaming_media_io_state&) = delete;

    _streaming_media_io_state& operator=(const _streaming_media_io_state&) = delete;
   
    int stream_number;

    MediaSubsession* live_subsession;

    _streaming_media_session_impl* _streaming_media_session;

    media_sample sample;
};

_streaming_media_session_impl::_streaming_media_session_impl(const media_session& session, 
                                                             std::vector<media_subsession> subsessions) :
    _session(session),
    _subsessions(std::move(subsessions))
{
    int stream_number = 0;

    for (auto& subsession : _subsessions)
    {
        auto live_subsession = subsession.__media_subsession->live_media_subsession();

        live_subsession->miscPtr = nullptr;

        FramedSource* framed_source = live_subsession->readSource();

        if(!framed_source)
        {
            continue;
        }

       
        if (live_subsession->rtcpInstance()) 
        {
            media_sample sample;

            sample.__media_sample_impl->capacity_set(1024 * 200);

            sample.__media_sample_impl->stream_number_set(stream_number);

            sample.__media_sample_impl->codec_name_set(live_subsession->codecName());

            assert(!live_subsession->miscPtr);

            live_subsession->miscPtr = new _streaming_media_io_state{stream_number, live_subsession, this, sample};

            /* set a 'BYE' handler for this subsession's RTCP instance: */
            live_subsession->rtcpInstance()->setByeHandler(on_rtcp_bye, this);
        }

        stream_number++;
    }
}

void _streaming_media_session_impl::on_rtcp_bye(void* client_data)
{

}

_streaming_media_session_impl::~_streaming_media_session_impl()
{
    for (auto& subsession : _subsessions)
    {
        auto live_subsession = subsession.__media_subsession->live_media_subsession();

        if (live_subsession->rtcpInstance()) 
        {
            if (live_subsession->miscPtr)
            {
                auto state = static_cast<_streaming_media_io_state*>(live_subsession->miscPtr);

                delete state;

                live_subsession->miscPtr = nullptr;
            }

            FramedSource* subsessionSource = live_subsession->readSource();

            if (subsessionSource)
            {
                subsessionSource->stopGettingFrames();
            }

            live_subsession->rtcpInstance()->setByeHandler(on_rtcp_bye, nullptr);
        }
    }
}

void uvxx::rtsp::details::_streaming_media_session_impl::on_frame_callback_set(std::function<bool(const media_sample&)> callback)
{
    _on_frame_callback = std::move(callback);

    continue_reading();
}

void uvxx::rtsp::details::_streaming_media_session_impl::continue_reading()
{
    for (auto& subsession : _subsessions)
    {
        auto live_subsession = subsession.__media_subsession->live_media_subsession();

        FramedSource* framed_source = live_subsession->readSource();

        if (!framed_source)
        {
            continue;
        }

        if (framed_source->isCurrentlyAwaitingData())
        {
            continue;
        }

        auto state = static_cast<_streaming_media_io_state*>(live_subsession->miscPtr);

        framed_source->getNextFrame((unsigned char*)state->sample.__media_sample_impl->data(), 
                                    state->sample.__media_sample_impl->capacity(), 
                                    on_after_getting_frame, 
                                    live_subsession->miscPtr, 
                                    nullptr, 
                                    nullptr);
    }
}

void _streaming_media_session_impl::adjust_buffer_for_trucated_bytes(unsigned truncated_amount, const media_sample& sample)
{
    static const size_t MAX_BUFFER_SIZE = 2 * 1024 * 1024;

    if (truncated_amount == 0)
    {
        return;
    }

    size_t current_size = sample.__media_sample_impl->capacity();

    size_t new_size = current_size + (truncated_amount * 2);

    new_size = max(MAX_BUFFER_SIZE, new_size);

    if (new_size == current_size)
    {
        return;
    }

    printf("resizing buffer to %u\n", new_size);

    sample.__media_sample_impl->capacity_set(new_size);
}

void _streaming_media_session_impl::on_after_getting_frame(void* client_data, 
                                                           unsigned packet_data_size, 
                                                           unsigned truncated_bytes, 
                                                           struct timeval presentation_time, 
                                                           unsigned duration_in_microseconds)
{
    static uint64_t ONE_MILLION = 1000000ull;

    auto io_state = static_cast<_streaming_media_io_state*>(client_data);

    FramedSource* framed_source = io_state->live_subsession->readSource();

    auto& sample = io_state->sample;

    auto& sample_impl = sample.__media_sample_impl;

    if (truncated_bytes)
    {
        io_state->_streaming_media_session->adjust_buffer_for_trucated_bytes(truncated_bytes, sample);
    }

    std::chrono::microseconds micro_seconds((ONE_MILLION * presentation_time.tv_sec) + 
                                            presentation_time.tv_usec);

   
    sample_impl->presentation_time_set(micro_seconds);

    sample_impl->is_truncated_set(truncated_bytes > 0);

    sample_impl->size_set(packet_data_size);

    bool is_complete_sample = true;

    bool is_synced = true;

    if (framed_source->isRTPSource())
    {
        auto rtp_source = static_cast<RTPSource*>(framed_source);

        is_synced = rtp_source->hasBeenSynchronizedUsingRTCP();

        is_complete_sample = rtp_source->curPacketMarkerBit();

        sample_impl->is_complete_sample_set(is_complete_sample);
    }
    else
    {
        sample_impl->is_complete_sample_set(true);
    }

    if (sample_impl->codec_name() == "H264")
    {
        Parse((byte*)sample_impl->data(), packet_data_size);
    }

    bool is_iframe = isH264iFrame((byte*)sample_impl->data());

    if (is_iframe)
    {
        printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!! iframe\n");
    }
    
    bool continue_reading = io_state->_streaming_media_session->_on_frame_callback(sample);

    if (continue_reading)
    {
        io_state->_streaming_media_session->continue_reading();
    }
}

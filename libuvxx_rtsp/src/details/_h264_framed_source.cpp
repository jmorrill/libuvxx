#include "details/_h264_framed_source.hpp"
#include "media_sample.hpp"

using namespace uvxx::rtsp;
using namespace uvxx::rtsp::details;

_h264_framed_source::_h264_framed_source(int stream_id, unsigned session_id): 
    _live_framed_source(stream_id, session_id), 
    _nal_to_deliver(nal_to_deliver::none),
    _payload(400000), 
    _payload_size(0),
    _start_time(std::chrono::high_resolution_clock::now())
{
}

_h264_framed_source::~_h264_framed_source()
{
    
}

void _h264_framed_source::deliver_sample_override(const media_sample& sample)
{
    if (_busy_delivering)
    {
        return;
    }

    fDurationInMicroseconds = 0;

    memcpy(_payload.data(), const_cast<unsigned char*>(sample.data()), sample.size());

    _payload_size = sample.size();

    bool is_key_frame = sample.attribute_get<bool>(sample_attributes::ATTRIBUTE_VIDEO_KEYFRAME);

    auto now = std::chrono::high_resolution_clock::now();

    auto time_since_start = std::chrono::duration_cast<std::chrono::seconds>(now - _start_time);

    static std::chrono::seconds one_second(1);

    if(is_key_frame || time_since_start > one_second)
    {
        _sps = sample.attribute_blob_get(sample_attributes::ATTRIBUTE_H26X_SEQUENCE_PARAMETER_SET);

        _pps = sample.attribute_blob_get(sample_attributes::ATTRIBUTE_H26X_PICTURE_PARAMETER_SET);

        _nal_to_deliver = nal_to_deliver::sps;
    }
    else
    {
        _nal_to_deliver = nal_to_deliver::payload;
    }

    _busy_delivering = true;

    do_iterative_nal_delivery();
}

#undef min

void _h264_framed_source::do_iterative_nal_delivery()
{
    if (_nal_to_deliver == nal_to_deliver::none)
    {
        return;
    }

    bool has_sample_to_deliver = false;

    switch (_nal_to_deliver)
    {
        case nal_to_deliver::sps:
        {
            auto sps_length = _sps.length_get();

            if(sps_length)
            {
                size_t bytes_to_copy = std::min(fMaxSize, sps_length);

                fNumTruncatedBytes = sps_length - bytes_to_copy;

                memcpy(fTo, _sps.data(), bytes_to_copy);

                fFrameSize = bytes_to_copy;

                _nal_to_deliver = nal_to_deliver::pps;

                has_sample_to_deliver = true;

                break;
            }
            /* else, fall through next case */
        }
        case nal_to_deliver::pps:
        {
            auto pps_length = _pps.length_get();

            if (pps_length)
            {
                size_t bytes_to_copy = std::min(fMaxSize, pps_length);

                fNumTruncatedBytes = pps_length - bytes_to_copy;

                memcpy(fTo, _pps.data(), bytes_to_copy);

                fFrameSize = bytes_to_copy;

                _nal_to_deliver = nal_to_deliver::payload;

                has_sample_to_deliver = true;

                break;
            }
            /* else, fall through next case */
        }
        case nal_to_deliver::payload:
        {
            if(_payload_size)
            {
                size_t bytes_to_copy = std::min(fMaxSize, _payload_size);

                fNumTruncatedBytes = _payload_size - bytes_to_copy;

                memcpy(fTo, _payload.data(), bytes_to_copy);

                fFrameSize = bytes_to_copy;

                _nal_to_deliver = nal_to_deliver::none;

                has_sample_to_deliver = true;
            }
           
            _busy_delivering = false;

            break;
        }
        case nal_to_deliver::none: 
    default: break;
    }
            
    if(has_sample_to_deliver)
    {
        FramedSource::afterGetting(this);
    }
}

void _h264_framed_source::doGetNextFrame()
{
    do_iterative_nal_delivery();
}
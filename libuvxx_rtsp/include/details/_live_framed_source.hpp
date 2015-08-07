#pragma once
#include "FramedSource.hh"

#include "event_dispatcher_object.hpp"
#include "media_descriptor.hpp"
#include "media_sample.hpp"
#include "sample_attributes.hpp"
#include "media_sample.hpp"

namespace uvxx { namespace rtsp 
{
    class media_sample;
    class media_descriptor;
}}

namespace uvxx { namespace rtsp { namespace details
{
    using _framed_source_closed_delegate = std::function<void(int stream_id)>;

    class _live_framed_source : public FramedSource
    {
    public:
        explicit _live_framed_source(int stream_id);

        virtual ~_live_framed_source();

    public:
        void on_closed_set(_framed_source_closed_delegate source_closed);

        void deliver_sample(const media_sample& sample)
        {
            if (!isCurrentlyAwaitingData() || _sample != nullptr)
                return;

            _sample = sample;

            stage = 0;

            memcpy(_payload, const_cast<unsigned char*>(_sample.data()), _sample.size());

            _payload_size = _sample.size();

            _sps = _sample.attribute_blob_get(sample_attributes::ATTRIBUTE_H26X_SEQUENCE_PARAMETER_SET);

            _pps = _sample.attribute_blob_get(sample_attributes::ATTRIBUTE_H26X_PICTURE_PARAMETER_SET);

            fDurationInMicroseconds = 0;

            fPresentationTime.tv_sec = static_cast<long>(sample.presentation_time().count() / 1000000);
            fPresentationTime.tv_usec = static_cast<long>(sample.presentation_time().count() % 1000000);

            doStage();
        }
    private:

        void doStage()
        {
            if (stage > 2 || _sample.size() == 0 || !isCurrentlyAwaitingData())
            {
                _sample = nullptr;
                return;
            }

            switch (stage)
            {
                case 0:
                {
                   
                    memcpy(fTo, _sps.data(), _sps.length_get());
                    fFrameSize = _sps.length_get();
                    break;
                }
                case 1:
                {
                    memcpy(fTo, _pps.data(), _pps.length_get());
                    fFrameSize = _pps.length_get();
                    break;
                }
                case 2:
                {
                    memcpy(fTo, _payload, _payload_size);
                    fFrameSize = _payload_size;
                    break;
                }
            }
            stage++;
            FramedSource::afterGetting(this);
        }

        virtual void doGetNextFrame() override
        {
            doStage();
        }

    private:
        unsigned char* _payload;

        int _payload_size;

        int stage = 0;

        media_sample _sample;

        uvxx::io::memory_buffer _sps;

        uvxx::io::memory_buffer _pps;

        _framed_source_closed_delegate _on_source_closed;

        int _stream_id;
    };
}}}
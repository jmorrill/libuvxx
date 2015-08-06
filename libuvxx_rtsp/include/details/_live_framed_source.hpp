#pragma once
#include "event_dispatcher_object.hpp"
#include "media_descriptor.hpp"
#include "media_sample.hpp"
#include "FramedSource.hh"
#include "sample_attributes.hpp"
#include "media_sample.hpp"
#include "details/_live_common.hpp"
#include "details/_live_framed_source.hpp"

namespace uvxx { namespace rtsp 
{
    class media_sample;
    class media_descriptor;
}}

namespace uvxx { namespace rtsp { namespace details
{
    class _live_framed_source : public FramedSource
    {
    public:
        explicit _live_framed_source(std::function<void()> source_closed) : FramedSource(*_get_live_environment().get()),
            _closed(source_closed)
        {
            buf = new unsigned char[400000];
            fTo = nullptr;
        }

        virtual ~_live_framed_source()
        {
            if (_closed)
            {
                _closed();
            }
        }
        unsigned char* buf;
        int fbuffsize;
        int stage = 0;
        media_sample _sample;
    public:

        virtual void doStopGettingFrames() override
        {

        }
        void deliver_sample(const media_sample& sample)
        {
            if (!isCurrentlyAwaitingData() || _sample != nullptr)
                return;

            _sample = sample;
            stage = 0;
            memcpy(buf, const_cast<unsigned char*>(_sample.data()), _sample.size());
            fbuffsize = _sample.size();
            fDurationInMicroseconds = 0;
            fFrameSize = sample.size();

            fPresentationTime.tv_sec = static_cast<long>(sample.presentation_time().count() / 1000000);
            fPresentationTime.tv_usec = static_cast<long>(sample.presentation_time().count() % 100000);

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
                auto sps = _sample.attribute_blob_get(sample_attributes::ATTRIBUTE_H26X_SEQUENCE_PARAMETER_SET);
                memcpy(fTo, sps.data(), sps.length_get());
                fFrameSize = sps.length_get();
                break;
            }
            case 1:
            {
                auto pps = _sample.attribute_blob_get(sample_attributes::ATTRIBUTE_H26X_PICTURE_PARAMETER_SET);
                memcpy(fTo, pps.data(), pps.length_get());
                fFrameSize = pps.length_get();
                break;
            }
            case 2:
            {
                memcpy(fTo, buf, fbuffsize);
                fFrameSize = fbuffsize;
                break;
            }
            }
            stage++;
            FramedSource::afterGetting(this);
        }
        virtual void doGetNextFrame() override
        {
            doStage();
            printf("do get next\n");
        }

    private:
        std::function<void()> _closed;
    };
}}}
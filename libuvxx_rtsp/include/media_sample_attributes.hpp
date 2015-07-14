#pragma once

namespace uvxx { namespace rtsp { namespace media_sample_attributes {

    struct video_dimensions
    {
        int width; int height;
    };

    enum class media_sample_majortype
    {
        unknown = 0,
        video = 1,
        audio = 2
    };

    static const char* SAMPLE_MAJOR_TYPE = "media_sample.majortype";

    /* video attributes */
    static const char* VIDEO_DIMENSIONS = "media_sample.video.dimensions";
    static const char* VIDEO_KEYFRAME = "media_sample.video.keyframe";

    /* h26x attributes */
    static const char* H26X_SEQUENCE_PARAMETER_SET = "media_sample.h26x.sps";
    static const char* H26X_PICTURE_PARAMETER_SET = "media_sample.h26x.pps";
}}}
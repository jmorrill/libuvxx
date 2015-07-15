#pragma once
#include <string>

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

    static const std::string ATTRIBUTE_SAMPLE_MAJOR_TYPE = "media_sample.majortype";

    /* video attributes */
    static const std::string ATTRIBUTE_VIDEO_DIMENSIONS = "media_sample.video.dimensions";
    static const std::string ATTRIBUTE_VIDEO_KEYFRAME = "media_sample.video.keyframe";

    /* h26x attributes */
    static const std::string ATTRIBUTE_H26X_SEQUENCE_PARAMETER_SET = "media_sample.h26x.sps";
    static const std::string ATTRIBUTE_H26X_PICTURE_PARAMETER_SET = "media_sample.h26x.pps";
}}}
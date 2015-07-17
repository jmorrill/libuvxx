#pragma once
#include <string>

namespace uvxx { namespace rtsp { namespace media_attributes {

    struct video_dimensions
    {
        int width; int height;
    };

    enum class media_sample_major_type
    {
        unknown = 0,
        video = 1,
        audio = 2
    };

    /* common attributes */
    static const std::string ATTRIBUTE_SAMPLE_MAJOR_TYPE = "sample.majortype";

    /* video attributes */
    static const std::string ATTRIBUTE_VIDEO_DIMENSIONS = "sample.video.dimensions";
    static const std::string ATTRIBUTE_VIDEO_KEYFRAME = "sample.video.keyframe";

    /* h26x attributes */
    static const std::string ATTRIBUTE_H26X_SEQUENCE_PARAMETER_SET = "sample.h26x.sps";
    static const std::string ATTRIBUTE_H26X_PICTURE_PARAMETER_SET = "sample.h26x.pps";
}}}
#pragma once

#include <string>

#include "gst/gst.h"

template<typename T>
decltype(auto) g_object_cast(T value) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
    return G_OBJECT(value);
#pragma GCC diagnostic pop
}

template<typename T>
decltype(auto) gst_object_cast(T value) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
    return GST_OBJECT(value);
#pragma GCC diagnostic pop
}

template<typename T>
decltype(auto) gst_bin_cast(T value) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
    return GST_BIN(value);
#pragma GCC diagnostic pop
}

template<typename T>
decltype(auto) gst_video_overlay_cast(T value) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
    return GST_VIDEO_OVERLAY(value);
#pragma GCC diagnostic pop
}

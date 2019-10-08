find_package(PkgConfig)
pkg_check_modules(GST REQUIRED gstreamer-1.0>=1.4 gstreamer-video-1.0)

add_library(gstreamer INTERFACE)
target_include_directories(gstreamer SYSTEM INTERFACE ${GST_INCLUDE_DIRS})
target_compile_options(gstreamer INTERFACE ${GST_CFLAGS})
target_link_libraries(gstreamer INTERFACE ${GST_LIBRARIES})

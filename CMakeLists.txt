cmake_minimum_required(VERSION 3.7)

project(dingdong CXX)

# Note that we cannot use C++17, because Raspbian currently has GCC 6.3.
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Werror -Wnon-virtual-dtor -Wold-style-cast")

include(cmake/clang_tidy.cmake)
include(cmake/gstreamer.cmake)

set(HEADERS
    src/stream/audio.h
    src/stream/gstreamer.h
    src/stream/gstreamer_helpers.h
    src/stream/video.h)

set(SOURCES
    src/stream/audio.cpp
    src/stream/gstreamer.cpp
    src/stream/video.cpp
    src/test.cpp)

set(CONFIG_FILES
    .clang-tidy)

add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES} ${CONFIG_FILES})
target_link_libraries(${PROJECT_NAME} gstreamer)

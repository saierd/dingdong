cmake_minimum_required(VERSION 3.7)

project(dingdong CXX)

# Note that we cannot use C++17, because Raspbian currently has GCC 6.3.
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Werror -Wnon-virtual-dtor -Wold-style-cast")

include(cmake/clang_tidy.cmake)
include(cmake/gstreamer.cmake)
include(cmake/gtkmm.cmake)

set(HEADERS
    src/discovery.h
    src/instance.h
    src/network/interface.h
    src/network/ip_address.h
    src/network/udp.h
    src/settings.h
    src/stream/audio.h
    src/stream/gstreamer.h
    src/stream/gstreamer_helpers.h
    src/stream/video.h
    src/system/machine_id.h
    src/ui/main_window.h)

set(SOURCES
    src/discovery.cpp
    src/main.cpp
    src/network/interface.cpp
    src/network/ip_address.cpp
    src/network/udp.cpp
    src/settings.cpp
    src/stream/audio.cpp
    src/stream/gstreamer.cpp
    src/stream/video.cpp
    src/system/machine_id.cpp
    src/ui/main_window.cpp)

set(CONFIG_FILES
    .clang-tidy)

add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES} ${CONFIG_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE src)
target_link_libraries(${PROJECT_NAME} PRIVATE gstreamer gtkmm pthread)

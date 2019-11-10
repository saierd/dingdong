cmake_minimum_required(VERSION 3.10)

project(dingdong C CXX)

set(RASPBERRY_PI OFF)
if ("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "^arm")
    set(RASPBERRY_PI ON)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wdouble-promotion -Wnon-virtual-dtor -Wnull-dereference -Wold-style-cast -Wshadow")

include(cmake/clang_tidy.cmake)
include(cmake/ccache.cmake)
include(cmake/gresource.cmake)
include(cmake/gstreamer.cmake)
include(cmake/gtkmm.cmake)
include(cmake/openssl.cmake)
include(cmake/pulseaudio.cmake)
include(cmake/uuid.cmake)
include(cmake/x11.cmake)
include(cmake/xfixes.cmake)

find_package(Threads REQUIRED)

add_subdirectory(3rdparty)

set(HEADERS
    src/access_control/access_control.h
    src/access_control/action.h
    src/access_control/actions/callback.h
    src/access_control/actions/gpio.h
    src/access_control/key.h
    src/access_control/rfid_scanner.h
    src/audio_player.h
    src/call.h
    src/call_protocol.h
    src/discovery.h
    src/gstreamer/gstreamer.h
    src/gstreamer/gstreamer_helpers.h
    src/gstreamer/pipeline.h
    src/instance.h
    src/network/interface.h
    src/network/ip_address.h
    src/network/udp.h
    src/pulseaudio_sink_occupier.h
    src/screen_control.h
    src/settings.h
    src/stream/audio.h
    src/stream/video.h
    src/system/beep.h
    src/system/gpio.h
    src/system/machine_id.h
    src/system/uuid.h
    src/ui/constants.h
    src/ui/gtk_helpers.h
    src/ui/main_window.h
    src/ui/screen.h
    src/ui/screens/action_screen.h
    src/ui/screens/call_screen.h
    src/ui/screens/key_screen.h
    src/ui/screens/key_input_screen.h
    src/ui/screens/main_screen.h
    src/ui/x_helpers.h
    src/util/json.h
    src/util/logging.h)

set(SOURCES
    src/access_control/access_control.cpp
    src/access_control/actions/callback.cpp
    src/access_control/actions/gpio.cpp
    src/access_control/key.cpp
    src/access_control/rfid_scanner.cpp
    src/audio_player.cpp
    src/call.cpp
    src/call_protocol.cpp
    src/discovery.cpp
    src/gstreamer/gstreamer.cpp
    src/gstreamer/pipeline.cpp
    src/main.cpp
    src/network/interface.cpp
    src/network/ip_address.cpp
    src/network/udp.cpp
    src/pulseaudio_sink_occupier.cpp
    src/screen_control.cpp
    src/settings.cpp
    src/stream/audio.cpp
    src/stream/video.cpp
    src/system/beep.cpp
    src/system/gpio.cpp
    src/system/machine_id.cpp
    src/system/uuid.cpp
    src/ui/main_window.cpp
    src/ui/screens/action_screen.cpp
    src/ui/screens/call_screen.cpp
    src/ui/screens/key_screen.cpp
    src/ui/screens/key_input_screen.cpp
    src/ui/screens/main_screen.cpp
    src/ui/x_helpers.cpp
    src/util/logging.cpp)

set(CONFIG_FILES
    .clang-format
    .clang-tidy)

compile_gresource(resources/resources.xml RESOURCE_FILE)

add_executable(${PROJECT_NAME}
    ${HEADERS}
    ${SOURCES}
    resources/resources.xml
    ${RESOURCE_FILE}
    ${CONFIG_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE
    src)
target_link_libraries(${PROJECT_NAME} PRIVATE
    cpp-httplib
    gstreamer
    gtkmm
    libuuid
    json
    openssl
    pulseaudio
    procxx
    spdlog
    Threads::Threads
    x11
    xfixes)

if (RASPBERRY_PI)
    target_compile_definitions(${PROJECT_NAME} PRIVATE RASPBERRY_PI)
endif()

if (RASPBERRY_PI)
    include(install/package.cmake)
endif()

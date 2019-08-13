option(SPDLOG_BUILD_TESTING OFF)
add_subdirectory(spdlog)

option(JSON_BuildTests OFF)
add_subdirectory(json)

add_library(cpp-httplib INTERFACE)
target_include_directories(cpp-httplib SYSTEM INTERFACE cpp-httplib)

# pigpio's CMakeLists.txt is pretty bad at the moment and builds all kind of stuff. The sources of
# the following library are taken from its pigpio target.
add_library(pigpio STATIC
    pigpio/pigpiod_if2.c
    pigpio/pigpiod_if2.h
    pigpio/command.c
    pigpio/command.h)
target_include_directories(pigpio SYSTEM PUBLIC pigpio)
target_compile_options(pigpio PRIVATE "-w")

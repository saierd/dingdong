set(SPDLOG_BUILD_TESTING OFF)
add_subdirectory(spdlog)

set(JSON_BuildTests OFF)
add_subdirectory(json)

add_library(cpp-httplib INTERFACE)
target_include_directories(cpp-httplib SYSTEM INTERFACE cpp-httplib)

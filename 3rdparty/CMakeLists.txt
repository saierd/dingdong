option(SPDLOG_BUILD_TESTING OFF)
add_subdirectory(spdlog EXCLUDE_FROM_ALL)

add_library(json INTERFACE)
target_include_directories(json SYSTEM INTERFACE json)
if (RASPBERRY_PI)
    # Avoid a warning in the json library with GCC on Raspberry.
    # See https://github.com/nlohmann/json/issues/658
    target_compile_options(json INTERFACE "-Wno-psabi")
endif()

add_library(cpp-httplib INTERFACE)
target_include_directories(cpp-httplib SYSTEM INTERFACE cpp-httplib)

add_library(spdlog INTERFACE)
target_include_directories(spdlog SYSTEM INTERFACE "spdlog/include")

add_library(json INTERFACE)
target_include_directories(json SYSTEM INTERFACE json)
if (RASPBERRY_PI)
    # Avoid a warning in the json library with GCC on Raspberry.
    # See https://github.com/nlohmann/json/issues/658
    target_compile_options(json INTERFACE "-Wno-psabi")
endif()

add_library(cpp-httplib INTERFACE)
target_include_directories(cpp-httplib SYSTEM INTERFACE cpp-httplib)

add_library(procxx INTERFACE)
target_include_directories(procxx SYSTEM INTERFACE procxx)

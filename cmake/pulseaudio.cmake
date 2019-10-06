find_package(PkgConfig)
pkg_check_modules(PA_SIMPLE REQUIRED libpulse-simple)

add_library(pulseaudio INTERFACE)
target_include_directories(pulseaudio SYSTEM INTERFACE ${PA_SIMPLE_INCLUDE_DIRS})
target_compile_options(pulseaudio INTERFACE ${PA_SIMPLE_CFLAGS})
target_link_libraries(pulseaudio INTERFACE ${PA_SIMPLE_LIBRARIES})

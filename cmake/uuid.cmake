find_package(PkgConfig)
pkg_check_modules(UUID REQUIRED uuid)

add_library(libuuid INTERFACE)
target_include_directories(libuuid SYSTEM INTERFACE ${UUID_INCLUDE_DIRS})
target_link_libraries(libuuid INTERFACE ${UUID_LIBRARIES})

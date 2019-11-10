find_package(PkgConfig)
pkg_check_modules(X11 REQUIRED x11>=1.6)

add_library(x11 INTERFACE)
target_include_directories(x11 SYSTEM INTERFACE ${X11_INCLUDE_DIRS})
target_compile_options(x11 INTERFACE ${X11_CFLAGS})
target_link_libraries(x11 INTERFACE ${X11_LIBRARIES})

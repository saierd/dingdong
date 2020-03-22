find_package(PkgConfig)
pkg_check_modules(GDK_X11 REQUIRED gdk-x11-3.0)

add_library(gdk-x11 INTERFACE)
target_include_directories(gdk-x11 SYSTEM INTERFACE ${GDK_X11_INCLUDE_DIRS})
target_link_libraries(gdk-x11 INTERFACE ${GDK_X11_LIBRARIES})

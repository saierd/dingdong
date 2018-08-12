find_package(PkgConfig)
pkg_check_modules(GTKMM REQUIRED gtkmm-3.0)

add_library(gtkmm INTERFACE)
target_include_directories(gtkmm SYSTEM INTERFACE ${GTKMM_INCLUDE_DIRS})
target_link_libraries(gtkmm INTERFACE ${GTKMM_LIBRARIES})

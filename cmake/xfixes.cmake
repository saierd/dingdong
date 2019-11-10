find_package(PkgConfig)
pkg_check_modules(XFIXES REQUIRED xfixes>=5.0)

add_library(xfixes INTERFACE)
target_include_directories(xfixes SYSTEM INTERFACE ${XFIXES_INCLUDE_DIRS})
target_compile_options(xfixes INTERFACE ${XFIXES_CFLAGS})
target_link_libraries(xfixes INTERFACE ${XFIXES_LIBRARIES})

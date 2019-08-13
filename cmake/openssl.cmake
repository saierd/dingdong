find_package(PkgConfig)
pkg_check_modules(OPENSSL REQUIRED openssl)

add_library(openssl INTERFACE)
target_include_directories(openssl SYSTEM INTERFACE ${OPENSSL_INCLUDE_DIRS})
target_link_libraries(openssl INTERFACE ${OPENSSL_LIBRARIES})

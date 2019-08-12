set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(TOOLCHAIN_PREFIX arm-linux-gnueabihf)

set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}-g++)

set(CMAKE_FIND_ROOT_PATH /usr/${TOOLCHAIN_PREFIX})

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Second entry is the fallback for cross-architecture packages like xproto. These are only
# installed once, in the global pkgconfig directory and we have to fall back to them when there
# is no architecture specific version of the package.
set(ENV{PKG_CONFIG_LIBDIR} "/usr/lib/${TOOLCHAIN_PREFIX}/pkgconfig/:/usr/share/pkgconfig/")

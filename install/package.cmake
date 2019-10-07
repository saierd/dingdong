# Directory in which the program and any additional scripts will be installed.
set(INSTALL_DIRECTORY "/opt/dingdong")

set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_CONTACT "Daniel Saier")
set(CPACK_PACKAGE_VENDOR "SSSaier")

# Set the package architecture.
if (RASPBERRY_PI)
    set(CPACK_SYSTEM_NAME "armhf")
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "armhf")
else()
    set(CPACK_SYSTEM_NAME "amd64")
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
endif()

# Add install scripts to the package.
configure_file("install/conffiles.in" "${CMAKE_CURRENT_BINARY_DIR}/install/conffiles")
set(CPACK_DEBIAN_PACKAGE_CONTROL_STRICT_PERMISSION ON)
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
    "${CMAKE_CURRENT_SOURCE_DIR}/install/postinst"
    "${CMAKE_CURRENT_BINARY_DIR}/install/conffiles")

# Set up the package dependencies. Note that most dependencies will be detected automatically by dpkg-shlibdeps. The
# additional dependencies are required at runtime.
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_DEBIAN_PACKAGE_DEPENDS "gstreamer1.0-pulseaudio, pulseaudio, sox, wiringpi, x11-xserver-utils")

# Install binaries and scripts.
install(
    TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION "${INSTALL_DIRECTORY}/bin")
install(
    DIRECTORY "scripts"
    DESTINATION "${INSTALL_DIRECTORY}"
    USE_SOURCE_PERMISSIONS)

# Create a settings directory that is writable for the dingdong service.
install(
    DIRECTORY
    DESTINATION "${INSTALL_DIRECTORY}/settings"
    DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_WRITE WORLD_EXECUTE)

# Install the default settings file that can be adjusted by the user.
install(
    FILES "settings/settings.json.default"
    DESTINATION "${INSTALL_DIRECTORY}/settings"
    RENAME "settings.json"
    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ WORLD_WRITE)

# Install the systemd service description.
configure_file("install/dingdong.service.in" "${CMAKE_CURRENT_BINARY_DIR}/install/dingdong.service")
install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/install/dingdong.service"
    DESTINATION "/lib/systemd/system")

# This creates a target 'package' using the CPack settings above.
include(CPack)

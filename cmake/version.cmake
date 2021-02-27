find_package(Git REQUIRED)

execute_process(
    COMMAND ${GIT_EXECUTABLE} describe --tags --always
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE project_version)
string(STRIP "${project_version}" project_version)
string(REGEX REPLACE "^v" "" project_version "${project_version}")

message(STATUS "Version: ${project_version}")

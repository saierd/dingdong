find_program(GLIB_COMPILE_RESOURCES_EXE NAMES "glib-compile-resources")
if (NOT GLIB_COMPILE_RESOURCES_EXE)
    message(FATAL_ERROR "Could not find glib-compile-resources")
endif()

function(compile_gresource xml_file source_file)
    set(xml_file_absolute "${CMAKE_CURRENT_SOURCE_DIR}/${xml_file}")
    get_filename_component(xml_file_directory "${xml_file_absolute}" DIRECTORY)
    get_filename_component(compiled_file "${xml_file_absolute}" NAME)
    set(compiled_file "${CMAKE_CURRENT_BINARY_DIR}/${compiled_file}.c")

    add_custom_command(
        OUTPUT "${compiled_file}"
        COMMAND "${GLIB_COMPILE_RESOURCES_EXE}" --generate-source --target=${compiled_file} ${xml_file_absolute}
        # Note: This should also depend on the resource files, but we only depend on the xml for simplicity.
        DEPENDS ${xml_file_absolute}
        WORKING_DIRECTORY "${xml_file_directory}")

    set(${source_file} "${compiled_file}" PARENT_SCOPE)
endfunction()

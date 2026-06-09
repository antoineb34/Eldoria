# Shared CMake helpers for the Eldoria workspace.
#
# Keep target registration centralized here so adding new applications or
# modules does not turn the root CMakeLists.txt into a pile of one-off logic.

function(eldoria_add_module module_name module_path)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${module_path}/CMakeLists.txt")
        message(STATUS "Adding Eldoria module: ${module_name}")
        add_subdirectory("${module_path}")
    else()
        message(STATUS "Skipping Eldoria module ${module_name}: ${module_path} has no CMakeLists.txt yet")
    endif()
endfunction()

function(eldoria_add_app app_name app_path)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${app_path}/CMakeLists.txt")
        message(STATUS "Adding Eldoria app: ${app_name}")
        add_subdirectory("${app_path}")
    else()
        message(STATUS "Skipping Eldoria app ${app_name}: ${app_path} has no CMakeLists.txt yet")
    endif()
endfunction()

cmake_minimum_required(VERSION 3.29.3)

project(PirateJam VERSION 0.01)

set(CMAKE_EXECUTABLE_SUFFIX ".html")
add_compile_definitions(SOKOL_GLES3)
# add_compile_definitions(SOKOL_GLCORE)
# add_compile_options(-o cube-emsc.html)
add_link_options(-sUSE_WEBGL2 --shell-file=../shell.html)

file(GLOB source_files main.c)

# Sokol's GLSL compiler
set(GLSL_COMPILER "${CMAKE_SOURCE_DIR}/include/sokol/bin/sokol-shdc.exe")

# Find all GLSL files in the shaders directory
file(GLOB SHADER_SOURCES "${CMAKE_SOURCE_DIR}/res/shaders/*.glsl")
foreach(SHADER_SOURCE ${SHADER_SOURCES})
    get_filename_component(SHADER_NAME ${SHADER_SOURCE} NAME_WE)
    set(HEADER_FILE "${CMAKE_SOURCE_DIR}/res/compiled_shaders/${SHADER_NAME}.glsl.h")

    # Custom command to compile GLSL to header
    add_custom_command(
        OUTPUT ${HEADER_FILE}
        COMMAND ${GLSL_COMPILER} 
        ARGS -i ${SHADER_SOURCE} -o ${HEADER_FILE} -l glsl300es
        DEPENDS ${SHADER_SOURCE}
        COMMENT "Compiling ${SHADER_SOURCE} to ${HEADER_FILE}"
    )
    # set(COMPILED_SHADER_SOURCES "${COMPILED_SHADER_SOURCES} ${HEADER_FILE}")
    set_property(SOURCE main.c APPEND PROPERTY OBJECT_DEPENDS "${HEADER_FILE}")
    # Ensure the executable depends on the generated header
    # add_dependencies(${CMAKE_PROJECT_NAME} ${HEADER_FILE})
    # file(GLOB source_files "${source_files} ${HEADER_FILE}")
endforeach()

add_executable(
    ${CMAKE_PROJECT_NAME}
    ${source_files}
)

include_directories(include/)
include_directories(include/sokol)
include_directories(include/sokol/util)
include_directories(res/compiled_shaders)

add_subdirectory(res/shaders/)
add_subdirectory(res/compiled_shaders/)
add_subdirectory(src/)
# add_subdirectory(src)

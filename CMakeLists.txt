cmake_minimum_required(VERSION 4.3.3)

project(sol LANGUAGES C)

# Standards

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Gather sources

file(GLOB SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/sol/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/sol/common/*.c"
)

set(SOL_CLI_SOURCE
    "${CMAKE_CURRENT_SOURCE_DIR}/sol/sol.c"
)

list(REMOVE_ITEM SOURCES ${SOL_CLI_SOURCE})

# Libraries

add_library(sol_shared SHARED ${SOURCES})
add_library(sol_static STATIC ${SOURCES})

target_include_directories(sol_shared
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(sol_static
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set_target_properties(sol_shared PROPERTIES
    OUTPUT_NAME sol
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/embed
)

set_target_properties(sol_static PROPERTIES
    OUTPUT_NAME sol
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/embed
)

# CLI executable

add_executable(sol
    ${SOL_CLI_SOURCE}
)

target_include_directories(sol
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(sol
    PRIVATE
        sol_shared
)

set_target_properties(sol PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Copy public headers into build/embed

add_custom_command(
    TARGET sol_shared
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_BINARY_DIR}/embed
)

# object files under build/obj

set(CMAKE_OBJECT_PATH_MAX 4096)

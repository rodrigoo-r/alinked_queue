include(FetchContent)

cmake_minimum_required(VERSION 3.31)
project(alinked_queue C)

set(CMAKE_C_STANDARD 11)

add_library(alinked_queue STATIC alinked_queue.c alinked_queue.h)

if(NOT FLUENT_LIBC_RELEASE) # Manually add libraries only if not in release mode
    FetchContent_Declare(
            types
            GIT_REPOSITORY https://github.com/rodrigoo-r/types
            GIT_TAG        master
    )

    FetchContent_Declare(
            stdbool
            GIT_REPOSITORY https://github.com/rodrigoo-r/std_bool
            GIT_TAG        master
    )

    FetchContent_Declare(
            arena
            GIT_REPOSITORY https://github.com/rodrigoo-r/arena
            GIT_TAG        master
    )

    FetchContent_MakeAvailable(arena)
    FetchContent_MakeAvailable(types)
    FetchContent_MakeAvailable(stdbool)

    target_include_directories(alinked_queue PRIVATE ${CMAKE_BINARY_DIR}/_deps/types-src)
    target_include_directories(alinked_queue PRIVATE ${CMAKE_BINARY_DIR}/_deps/stdbool-src)
    target_include_directories(alinked_queue PRIVATE ${CMAKE_BINARY_DIR}/_deps/arena-src)
    target_include_directories(alinked_queue PRIVATE ${CMAKE_BINARY_DIR}/_deps/vector-src)
    target_link_libraries(alinked_queue PRIVATE types)
    target_link_libraries(alinked_queue PRIVATE stdbool)
    target_link_libraries(alinked_queue PRIVATE arena)
endif ()
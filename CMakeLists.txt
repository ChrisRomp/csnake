cmake_minimum_required(VERSION 3.10)
project(CSnake VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add executable
add_executable(snake snake.cpp)

# Platform-specific settings
if(UNIX AND NOT APPLE)
    # Linux-specific
    target_compile_options(snake PRIVATE -Wall -Wextra)
elseif(APPLE)
    # macOS-specific
    target_compile_options(snake PRIVATE -Wall -Wextra)
elseif(WIN32)
    # Windows-specific
    target_compile_options(snake PRIVATE /W4)
endif()

# Enable optimizations for Release build
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(MSVC)
        target_compile_options(snake PRIVATE /O2)
    else()
        target_compile_options(snake PRIVATE -O3)
    endif()
endif()

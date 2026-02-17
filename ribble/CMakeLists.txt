cmake_minimum_required(VERSION 3.20)

# Project metadata
project(Ribble
        VERSION 1.0.0
        DESCRIPTION "Ribble game engine library"
        LANGUAGES CXX C
)

# Only allow in-tree builds in specific cases
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "In-source builds are not allowed. Please create a separate build directory.")
endif()

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Generate compile_commands.json for IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build options
option(RIBBLE_BUILD_SHARED "Build Ribble as a shared library" OFF)
option(RIBBLE_BUILD_DEBUG "Build Ribble in debug mode (more logging)" ON)
option(RIBBLE_GLM_EXPERIMENTAL "Build Ribble with glm experimental features" ON)
option(RIBBLE_BUILD_TESTS "Build Ribble tests" ON)
option(RIBBLE_INSTALL "Enable install target" OFF)

# Platform detection
if(WIN32)
    set(RIBBLE_PLATFORM_WINDOWS TRUE)
elseif(APPLE)
    set(RIBBLE_PLATFORM_MACOS TRUE)
elseif(UNIX)
    set(RIBBLE_PLATFORM_LINUX TRUE)
endif()

# Compiler warnings
if(MSVC)
    add_compile_options(/W4 /WX-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Dependencies
include(FetchContent)

set(FETCHCONTENT_QUIET ON)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

if(RIBBLE_BUILD_TESTS)
    FetchContent_Declare(doctest
            GIT_REPOSITORY https://github.com/doctest/doctest.git
            GIT_TAG v2.4.12
            GIT_SHALLOW TRUE)
endif()

FetchContent_Declare(spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.13.0
        GIT_SHALLOW TRUE)

FetchContent_Declare(stb
        GIT_REPOSITORY https://github.com/nothings/stb.git
        GIT_TAG master
        GIT_SHALLOW TRUE)

FetchContent_Declare(miniaudio
        GIT_REPOSITORY https://github.com/mackron/miniaudio.git
        GIT_TAG master
        GIT_SHALLOW TRUE)

FetchContent_Declare(json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
        GIT_SHALLOW TRUE)

FetchContent_Declare(ufbx
        GIT_REPOSITORY https://github.com/ufbx/ufbx.git
        GIT_TAG v0.14.3
        GIT_SHALLOW TRUE)

FetchContent_Declare(glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 1.0.1
        GIT_SHALLOW TRUE)

if(RIBBLE_BUILD_TESTS)
    FetchContent_MakeAvailable(doctest)
endif()

FetchContent_MakeAvailable(spdlog stb miniaudio json ufbx glm)

# Find required system libraries
find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 REQUIRED)

# GLAD library
add_library(glad STATIC "${CMAKE_CURRENT_SOURCE_DIR}/lib/glad/src/glad.c")
target_compile_options(glad PRIVATE -w)
target_include_directories(glad SYSTEM PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/lib/glad/include")

# ufbx library wrapper
add_library(ufbx_lib STATIC ${ufbx_SOURCE_DIR}/ufbx.c)
target_compile_options(ufbx_lib PRIVATE -w)
target_include_directories(ufbx_lib SYSTEM PUBLIC ${ufbx_SOURCE_DIR})
if(MSVC)
    target_compile_options(ufbx_lib PRIVATE /W0)
else()
    target_compile_options(ufbx_lib PRIVATE -w)
endif()


set(RIBBLE_SOURCES
        include/ribble.h
        include/ribble/engine/engine.h
        include/ribble/engine/engine_config.h
        include/ribble/error/error.h
        include/ribble/error/verbose_error.h
        source/error/error.cpp
        source/error/verbose_error.cpp
)

# Set library type
if(RIBBLE_BUILD_SHARED)
    set(LIBRARY_BUILD_MODE SHARED)
else()
    set(LIBRARY_BUILD_MODE STATIC)
endif()

# Create the main library
add_library(Ribble ${LIBRARY_BUILD_MODE}
        ${RIBBLE_SOURCES}
)
add_library(Ribble::Ribble ALIAS Ribble)

set_target_properties(Ribble PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        POSITION_INDEPENDENT_CODE ON
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
)

target_include_directories(Ribble
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${stb_SOURCE_DIR}>
        $<BUILD_INTERFACE:${miniaudio_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(Ribble
        PUBLIC
        OpenGL::GL
        glfw
        glm::glm
        spdlog::spdlog
        nlohmann_json::nlohmann_json
        PRIVATE
        glad
        ufbx_lib
)

if(RIBBLE_PLATFORM_LINUX)
    target_link_libraries(Ribble PRIVATE dl pthread)
elseif(RIBBLE_PLATFORM_WINDOWS)
    target_link_libraries(Ribble PRIVATE opengl32)
endif()

if(RIBBLE_GLM_EXPERIMENTAL)
    target_compile_definitions(Ribble PUBLIC GLM_ENABLE_EXPERIMENTAL)
endif()

if(RIBBLE_BUILD_DEBUG)
    target_compile_definitions(Ribble PUBLIC RIBBLE_DEBUG)
endif()

if(RIBBLE_BUILD_SHARED)
    target_compile_definitions(Ribble PRIVATE RIBBLE_EXPORTS)
endif()

if(RIBBLE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Configuration summary
message(STATUS "Ribble configured successfully.")
message(STATUS "")
message(STATUS "Ribble Configuration Summary")
message(STATUS "============================")
message(STATUS "Version:              ${PROJECT_VERSION}")
message(STATUS "Build type:           ${CMAKE_BUILD_TYPE}")
message(STATUS "Library type:         ${LIBRARY_BUILD_MODE}")
message(STATUS "C++ Standard:         ${CMAKE_CXX_STANDARD}")
message(STATUS "Install prefix:       ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")
message(STATUS "Options:")
message(STATUS "  RIBBLE_BUILD_SHARED:        ${RIBBLE_BUILD_SHARED}")
message(STATUS "  RIBBLE_BUILD_DEBUG:         ${RIBBLE_BUILD_DEBUG}")
message(STATUS "  RIBBLE_GLM_EXPERIMENTAL:    ${RIBBLE_GLM_EXPERIMENTAL}")
message(STATUS "  RIBBLE_BUILD_TESTS:         ${RIBBLE_BUILD_TESTS}")
message(STATUS "  RIBBLE_INSTALL:             ${RIBBLE_INSTALL}")
message(STATUS "")
message(STATUS "Dependencies:")
message(STATUS "  OpenGL:    ${OPENGL_LIBRARIES}")
message(STATUS "  GLFW3:     Found")
message(STATUS "  GLM:       ${glm_VERSION}")
message(STATUS "  spdlog:    ${spdlog_VERSION}")
message(STATUS "============================")
message(STATUS "")
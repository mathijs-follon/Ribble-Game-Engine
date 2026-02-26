# SDL3 window backend module

# Use local SDL3 if available, otherwise try to find system installation
set(SDL3_LOCAL_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../../lib/sdl3)
if(EXISTS ${SDL3_LOCAL_PATH}/CMakeLists.txt)
    # Build SDL3 from local clone
    add_subdirectory(${SDL3_LOCAL_PATH} sdl3_build)
    set(SDL3_TARGET SDL3-static)
    set(SDL3_INCLUDE_DIR ${SDL3_LOCAL_PATH}/include)
else()
    # Try to find system SDL3
    find_package(SDL3 QUIET)
    if(NOT SDL3_FOUND)
        find_path(SDL3_INCLUDE_DIR SDL3/SDL.h)
        find_library(SDL3_LIBRARY SDL3)
        if(SDL3_INCLUDE_DIR AND SDL3_LIBRARY)
            set(SDL3_FOUND TRUE)
            set(SDL3_TARGET ${SDL3_LIBRARY})
        endif()
    else()
        set(SDL3_TARGET SDL3::SDL3)
    endif()
endif()

if(SDL3_TARGET)
    add_library(ribble_backend_window_sdl3 STATIC
        sdl3_window_backend.cpp
        sdl3_window_backend.h
    )

    target_include_directories(ribble_backend_window_sdl3 PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
            ../../../include/
        ${SDL3_INCLUDE_DIR}
    )

    target_link_libraries(ribble_backend_window_sdl3 PUBLIC
        ${SDL3_TARGET}
        ribble_backend_common
    )

    target_compile_features(ribble_backend_window_sdl3 PUBLIC cxx_std_23)
else()
    add_library(ribble_backend_window_sdl3 INTERFACE)
    message(WARNING "SDL3 not found, SDL3 window backend will not be available")
endif()

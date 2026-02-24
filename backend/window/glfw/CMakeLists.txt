# GLFW window backend module

# Use local GLFW if available, otherwise try to find system installation
set(GLFW_LOCAL_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../../lib/glfw)
if(EXISTS ${GLFW_LOCAL_PATH}/CMakeLists.txt)
    # Build GLFW from local clone
    add_subdirectory(${GLFW_LOCAL_PATH} glfw_build)
    set(GLFW_TARGET glfw)
    set(GLFW_INCLUDE_DIR ${GLFW_LOCAL_PATH}/include)
else()
    # Try to find system GLFW
    find_package(glfw3 QUIET)
    if(NOT glfw3_FOUND)
        find_path(GLFW_INCLUDE_DIR GLFW/glfw3.h)
        find_library(GLFW_LIBRARY glfw3 glfw)
        if(GLFW_INCLUDE_DIR AND GLFW_LIBRARY)
            set(glfw3_FOUND TRUE)
            set(GLFW_TARGET ${GLFW_LIBRARY})
        endif()
    else()
        set(GLFW_TARGET glfw)
    endif()
endif()

if(GLFW_TARGET)
    add_library(ribble_backend_window_glfw STATIC
        glfw_window_backend.cpp
        glfw_window_backend.h
    )

    target_include_directories(ribble_backend_window_glfw PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../../common
        ${GLFW_INCLUDE_DIR}
    )

    target_link_libraries(ribble_backend_window_glfw PUBLIC
        ${GLFW_TARGET}
        ribble_backend_common
    )

    target_compile_features(ribble_backend_window_glfw PUBLIC cxx_std_23)
else()
    add_library(ribble_backend_window_glfw INTERFACE)
    message(WARNING "GLFW not found, GLFW window backend will not be available")
endif()

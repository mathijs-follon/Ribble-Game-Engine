# X11 window backend module

if(UNIX AND NOT APPLE)
    find_package(X11 REQUIRED)

    add_library(ribble_backend_window_x11 STATIC
        x11_window_backend.cpp
        x11_window_backend.h
    )

    target_include_directories(ribble_backend_window_x11 PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
            ../../../include/
        ${X11_INCLUDE_DIR}
    )

    target_link_libraries(ribble_backend_window_x11 PUBLIC
        ${X11_LIBRARIES}
        ribble_backend_common
    )

    target_compile_features(ribble_backend_window_x11 PUBLIC cxx_std_23)

    # Enable Position Independent Code so this static library can be linked into shared libraries
    set_target_properties(ribble_backend_window_x11 PROPERTIES
        POSITION_INDEPENDENT_CODE ON
    )
else()
    add_library(ribble_backend_window_x11 INTERFACE)
    message(STATUS "X11 window backend skipped (Linux/Unix only)")
endif()

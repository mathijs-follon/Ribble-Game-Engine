# Win32 window backend module

if(WIN32)
    add_library(ribble_backend_window_win32 STATIC
        win32_window_backend.cpp
        win32_window_backend.h
    )

    target_include_directories(ribble_backend_window_win32 PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../../common
    )

    target_link_libraries(ribble_backend_window_win32 PUBLIC
        user32
        gdi32
        ribble_backend_common
    )

    target_compile_features(ribble_backend_window_win32 PUBLIC cxx_std_23)
else()
    add_library(ribble_backend_window_win32 INTERFACE)
    message(STATUS "Win32 window backend skipped (Windows-only)")
endif()

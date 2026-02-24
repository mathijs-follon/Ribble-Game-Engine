# Cocoa window backend module

if(APPLE)
    find_library(APPKIT_LIBRARY AppKit)
    find_library(COCOA_LIBRARY Cocoa)

    add_library(ribble_backend_window_cocoa STATIC
        cocoa_window_backend.cpp
        cocoa_window_backend.h
    )

    target_include_directories(ribble_backend_window_cocoa PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../../common
    )

    target_link_libraries(ribble_backend_window_cocoa PUBLIC
        ${APPKIT_LIBRARY}
        ${COCOA_LIBRARY}
        ribble_backend_common
    )

    target_compile_features(ribble_backend_window_cocoa PUBLIC cxx_std_23)
else()
    add_library(ribble_backend_window_cocoa INTERFACE)
    message(STATUS "Cocoa window backend skipped (macOS only)")
endif()

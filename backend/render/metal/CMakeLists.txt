# Metal render backend module

if(APPLE)
    find_library(METAL_LIBRARY Metal)
    find_library(QUARTZCORE_LIBRARY QuartzCore)
    find_library(APPKIT_LIBRARY AppKit)

    add_library(ribble_backend_metal STATIC
        metal_backend.cpp
        metal_backend.h
        metal_buffer.cpp
        metal_buffer.h
        metal_command.cpp
        metal_command.h
        metal_conversions.h
        metal_pipeline.cpp
        metal_pipeline.h
        metal_shader.cpp
        metal_shader.h
        metal_swapchain.cpp
        metal_swapchain.h
        metal_texture.cpp
        metal_texture.h
    )

    target_include_directories(ribble_backend_metal PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../../common
    )

    target_link_libraries(ribble_backend_metal PUBLIC
        ${METAL_LIBRARY}
        ${QUARTZCORE_LIBRARY}
        ${APPKIT_LIBRARY}
        ribble_backend_common
    )

    target_compile_features(ribble_backend_metal PUBLIC cxx_std_23)
else()
    # Metal is macOS/iOS-only, create empty target on other platforms
    add_library(ribble_backend_metal INTERFACE)
    message(STATUS "Metal backend skipped (Apple platforms only)")
endif()

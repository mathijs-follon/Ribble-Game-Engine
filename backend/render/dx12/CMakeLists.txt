# DirectX 12 render backend module

if(WIN32)
    add_library(ribble_backend_dx12 STATIC
        dx12_backend.cpp
        dx12_backend.h
        dx12_buffer.cpp
        dx12_buffer.h
        dx12_command.cpp
        dx12_command.h
        dx12_conversions.h
        dx12_memory.cpp
        dx12_memory.h
        dx12_pipeline.cpp
        dx12_pipeline.h
        dx12_shader.cpp
        dx12_shader.h
        dx12_swapchain.cpp
        dx12_swapchain.h
        dx12_sync.cpp
        dx12_sync.h
        dx12_texture.cpp
        dx12_texture.h
    )

    target_include_directories(ribble_backend_dx12 PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../../common
    )

    target_link_libraries(ribble_backend_dx12 PUBLIC
        d3d12
        dxgi
        d3dcompiler
        ribble_backend_common
    )

    target_compile_features(ribble_backend_dx12 PUBLIC cxx_std_23)
else()
    # DX12 is Windows-only, create empty target on other platforms
    add_library(ribble_backend_dx12 INTERFACE)
    message(STATUS "DirectX 12 backend skipped (Windows-only)")
endif()

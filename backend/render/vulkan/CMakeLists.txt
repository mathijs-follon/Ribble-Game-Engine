# Vulkan render backend module

find_package(Vulkan REQUIRED)

add_library(ribble_backend_vulkan STATIC
    vulkan_backend.cpp
    vulkan_backend.h
    vulkan_buffer.cpp
    vulkan_buffer.h
    vulkan_command.cpp
    vulkan_command.h
    vulkan_descriptor.cpp
    vulkan_descriptor.h
    vulkan_memory.cpp
    vulkan_memory.h
    vulkan_pipeline.cpp
    vulkan_pipeline.h
    vulkan_shader.cpp
    vulkan_shader.h
    vulkan_swapchain.cpp
    vulkan_swapchain.h
    vulkan_sync.cpp
    vulkan_sync.h
    vulkan_texture.cpp
    vulkan_texture.h
    vulkan_conversions.h
)

target_include_directories(ribble_backend_vulkan PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../common
    ${Vulkan_INCLUDE_DIRS}
)

target_link_libraries(ribble_backend_vulkan PUBLIC
    ${Vulkan_LIBRARIES}
    ribble_backend_common
)

target_compile_features(ribble_backend_vulkan PUBLIC cxx_std_23)

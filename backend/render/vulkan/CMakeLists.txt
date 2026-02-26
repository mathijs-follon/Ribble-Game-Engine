# Vulkan render backend module

find_package(Vulkan QUIET)
find_program(GLSLANG_VALIDATOR glslangValidator)
if(Vulkan_FOUND)

# Compile default shaders to SPIR-V
set(VULKAN_SHADERS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
set(VULKAN_SHADERS_BUILD ${CMAKE_CURRENT_BINARY_DIR}/vulkan_shaders)
file(MAKE_DIRECTORY ${VULKAN_SHADERS_BUILD})
if(GLSLANG_VALIDATOR)
    add_custom_command(OUTPUT ${VULKAN_SHADERS_BUILD}/default_vert_spv.h
        COMMAND ${GLSLANG_VALIDATOR} -V ${VULKAN_SHADERS_DIR}/default.vert -o ${VULKAN_SHADERS_BUILD}/default_vert.spv
        COMMAND ${CMAKE_COMMAND} -E echo "// Generated from default.vert" > ${VULKAN_SHADERS_BUILD}/default_vert_spv.h
        COMMAND ${CMAKE_COMMAND} -E chdir ${VULKAN_SHADERS_BUILD} sh -c "xxd -i default_vert.spv >> default_vert_spv.h"
        DEPENDS ${VULKAN_SHADERS_DIR}/default.vert
        COMMENT "Compiling default vertex shader")
    add_custom_command(OUTPUT ${VULKAN_SHADERS_BUILD}/default_frag_spv.h
        COMMAND ${GLSLANG_VALIDATOR} -V ${VULKAN_SHADERS_DIR}/default.frag -o ${VULKAN_SHADERS_BUILD}/default_frag.spv
        COMMAND ${CMAKE_COMMAND} -E echo "// Generated from default.frag" > ${VULKAN_SHADERS_BUILD}/default_frag_spv.h
        COMMAND ${CMAKE_COMMAND} -E chdir ${VULKAN_SHADERS_BUILD} sh -c "xxd -i default_frag.spv >> default_frag_spv.h"
        DEPENDS ${VULKAN_SHADERS_DIR}/default.frag
        COMMENT "Compiling default fragment shader")
    add_custom_target(vulkan_compile_shaders DEPENDS
        ${VULKAN_SHADERS_BUILD}/default_vert_spv.h
        ${VULKAN_SHADERS_BUILD}/default_frag_spv.h)
else()
    message(WARNING "glslangValidator not found - Vulkan default pipelines may not work")
    set(VULKAN_SHADERS_BUILD ${CMAKE_CURRENT_BINARY_DIR})
endif()

add_library(ribble_backend_vulkan STATIC
    vulkan_default_shaders.cpp
    vulkan_backend.cpp
    vulkan_backend.h
    vulkan_buffer.cpp
    vulkan_buffer.h
    vulkan_command.cpp
    vulkan_command.h
    vulkan_context_glfw.cpp
    vulkan_context_sdl3.cpp
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
    vulkan_context.h
    vulkan_default_shaders.h
    vulkan_context_glfw.h
    vulkan_context_sdl3.h
)

target_include_directories(ribble_backend_vulkan PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../common
    ${CMAKE_SOURCE_DIR}/include
    ${VULKAN_SHADERS_BUILD}
    ${Vulkan_INCLUDE_DIRS}
)

target_link_libraries(ribble_backend_vulkan PUBLIC
    ${Vulkan_LIBRARIES}
    ribble_backend_common
)

# Vulkan contexts need window backends for surface creation
if(SDL3_RIBBLE_LINK_TARGET)
    target_link_libraries(ribble_backend_vulkan PRIVATE ${SDL3_RIBBLE_LINK_TARGET})
elseif(TARGET SDL3::SDL3)
    target_link_libraries(ribble_backend_vulkan PRIVATE SDL3::SDL3)
elseif(TARGET SDL3-static)
    target_link_libraries(ribble_backend_vulkan PRIVATE SDL3-static)
endif()
target_link_libraries(ribble_backend_vulkan PRIVATE glfw)

target_compile_features(ribble_backend_vulkan PUBLIC cxx_std_23)

if(GLSLANG_VALIDATOR)
    add_dependencies(ribble_backend_vulkan vulkan_compile_shaders)
    target_compile_definitions(ribble_backend_vulkan PRIVATE GLSLANG_VALIDATOR_FOUND)
endif()

# Allow static lib to be linked into shared libs
set_target_properties(ribble_backend_vulkan PROPERTIES POSITION_INDEPENDENT_CODE ON)

else()
    message(STATUS "Vulkan not found - Vulkan backend will be skipped")
endif()

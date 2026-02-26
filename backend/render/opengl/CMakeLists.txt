# OpenGL render backend module

find_package(OpenGL REQUIRED)

# GLAD setup
set(GLAD_GENERATED_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../lib/glad/generated)
set(GLAD_INCLUDE_DIR ${GLAD_GENERATED_DIR}/include)
set(GLAD_SRC_DIR ${GLAD_GENERATED_DIR}/src)

# Check if GLAD files exist
if(NOT EXISTS ${GLAD_INCLUDE_DIR}/glad/gl.h)
    message(WARNING "GLAD files not found at ${GLAD_INCLUDE_DIR}/glad/gl.h")
    message(WARNING "Please generate GLAD files using: python3 -m glad --api=gl:core=4.6 --out-path=lib/glad/generated c --loader")
endif()

add_library(ribble_backend_opengl STATIC
    opengl_backend.cpp
    opengl_backend.h
    opengl_buffer.cpp
    opengl_buffer.h
    opengl_framebuffer.cpp
    opengl_framebuffer.h
    opengl_shader.cpp
    opengl_shader.h
    opengl_state.cpp
    opengl_state.h
    opengl_texture.cpp
    opengl_texture.h
    opengl_conversions.h
        opengl_context.h
        opengl_context_sdl3.h
        opengl_context_glfw.h
    ${GLAD_SRC_DIR}/gl.c
)

target_include_directories(ribble_backend_opengl PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../common
    ../../../include/
    ${GLAD_INCLUDE_DIR}
)

target_link_libraries(ribble_backend_opengl PUBLIC
    OpenGL::GL
    ribble_backend_common
    spdlog::spdlog
)

target_compile_features(ribble_backend_opengl PUBLIC cxx_std_23)

# Enable Position Independent Code so this static library can be linked into shared libraries
set_target_properties(ribble_backend_opengl PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

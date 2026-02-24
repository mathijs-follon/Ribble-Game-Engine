# OpenGL render backend module

find_package(OpenGL REQUIRED)

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
)

target_include_directories(ribble_backend_opengl PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../common
)

target_link_libraries(ribble_backend_opengl PUBLIC
    OpenGL::GL
    ribble_backend_common
)

target_compile_features(ribble_backend_opengl PUBLIC cxx_std_23)

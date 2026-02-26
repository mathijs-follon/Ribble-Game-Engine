# Backend common headers (interface library)

add_library(ribble_backend_common INTERFACE)

target_include_directories(ribble_backend_common INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../include
)

target_compile_features(ribble_backend_common INTERFACE cxx_std_23)




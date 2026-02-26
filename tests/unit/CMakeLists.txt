# Unit tests

add_executable(ribble_unit_tests
    test_backend_interfaces.cpp
    test_backend_types.cpp
    test_opengl_backend.cpp
    test_vulkan_backend.cpp
    test_dx12_backend.cpp
    test_metal_backend.cpp
    test_config.cpp
)

target_include_directories(ribble_unit_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
        ../../include/ribble/backend
)

target_link_libraries(ribble_unit_tests PRIVATE
    doctest::doctest
    Ribble
)

target_compile_features(ribble_unit_tests PRIVATE cxx_std_23)


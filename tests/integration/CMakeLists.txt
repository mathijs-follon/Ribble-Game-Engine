# Integration tests

add_executable(ribble_integration_tests
    test_backend_switching.cpp
    test_multi_backend.cpp
)

target_include_directories(ribble_integration_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
        ../../include/ribble/backend
)

target_link_libraries(ribble_integration_tests PRIVATE
    doctest::doctest
    Ribble
)

target_compile_features(ribble_integration_tests PRIVATE cxx_std_23)


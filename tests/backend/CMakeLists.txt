# Backend tests

add_executable(ribble_backend_tests
    test_backend_comparison.cpp
    test_backend_factory.cpp
)

target_include_directories(ribble_backend_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
        ../../include/ribble/backend
)

target_link_libraries(ribble_backend_tests PRIVATE
    doctest::doctest
    Ribble
    ribble_backend_common
)

target_compile_features(ribble_backend_tests PRIVATE cxx_std_23)


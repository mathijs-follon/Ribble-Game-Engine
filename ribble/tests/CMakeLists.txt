cmake_minimum_required(VERSION 4.2)
project(RibbleLibTests)


set(TEST_INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/include
)

set(TEST_LINK_LIBS
        doctest::doctest
        Ribble
)

# Test executables
add_executable(test_math test_math.cpp)
target_include_directories(test_math PRIVATE ${TEST_INCLUDE_DIRS})
target_link_libraries(test_math PRIVATE ${TEST_LINK_LIBS})
add_test(NAME MathTests COMMAND test_math)

add_executable(test_error test_error.cpp)
target_include_directories(test_error PRIVATE ${TEST_INCLUDE_DIRS})
target_link_libraries(test_error PRIVATE ${TEST_LINK_LIBS})
add_test(NAME ErrorTests COMMAND test_error)

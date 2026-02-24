cmake_minimum_required(VERSION 3.31.6)

add_executable(TestExample
    main.cpp
)

target_include_directories(TestExample PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(TestExample PRIVATE
    Ribble
)

# explicitly compile ribble with RIBBLE_DEBUG for debugging log statements!
target_compile_definitions(Ribble
        PUBLIC RIBBLE_DEBUG
)

if(UNIX)
    target_link_libraries(TestExample PRIVATE dl)
endif()
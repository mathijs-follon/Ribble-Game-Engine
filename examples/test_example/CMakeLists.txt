cmake_minimum_required(VERSION 4.2.3)

add_executable(TestExample
    main.cpp
)

target_include_directories(TestExample PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(TestExample PRIVATE
    Ribble
)

if(UNIX)
    target_link_libraries(TestExample PRIVATE dl)
endif()
# Editor executable that uses the Ribble library

add_executable(RibbleEditor
    source/main.cpp
    source/editor/editor_application.cpp
    source/editor/editor_window.cpp
    source/editor/editor_state.cpp
    source/editor/editor_config.cpp
    source/editor/editor_panel.cpp
    source/editor/scene_editor.cpp
    source/editor/editor_command.cpp
    source/editor/view_manager.cpp
    source/editor/project_manager.cpp
    source/editor/panels/hierarchy_panel.cpp
    source/editor/panels/inspector_panel.cpp
    source/editor/panels/viewport_panel.cpp
    source/editor/panels/asset_browser_panel.cpp
    source/editor/panels/project_dialog_panel.cpp
    source/editor/panels/profiler_panel.cpp
)

target_include_directories(RibbleEditor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/lib/imgui
    ${CMAKE_SOURCE_DIR}/lib/imgui/backends
)

# Link to the engine library and dependencies
target_link_libraries(RibbleEditor PRIVATE
    Ribble
    ribble_backend_common
)

# Link ImGui if available
if(TARGET imgui)
    target_link_libraries(RibbleEditor PRIVATE imgui)
else()
    # Build ImGui from local clone
    set(IMGUI_PATH ${CMAKE_SOURCE_DIR}/lib/imgui)
    if(EXISTS ${IMGUI_PATH}/CMakeLists.txt)
        add_subdirectory(${IMGUI_PATH} imgui_build)
        target_link_libraries(RibbleEditor PRIVATE imgui)
    endif()
endif()

target_compile_features(RibbleEditor PRIVATE cxx_std_23)

# Copy font resources to build directory
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/resources/font)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources/font/
         DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/resources/font/
         FILES_MATCHING PATTERN "*.ttf"
    )
endif()


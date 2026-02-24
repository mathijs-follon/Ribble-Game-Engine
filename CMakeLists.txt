cmake_minimum_required(VERSION 3.24)
project(Ribble)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Setup spdlog for logging
include(FetchContent)
# Configure spdlog to build as a shared library to avoid TLS issues when linking into shared libraries
set(SPDLOG_BUILD_SHARED ON CACHE BOOL "Build spdlog as a shared library" FORCE)
set(SPDLOG_BUILD_EXAMPLE OFF CACHE BOOL "Build spdlog examples" FORCE)
set(SPDLOG_BUILD_TESTS OFF CACHE BOOL "Build spdlog tests" FORCE)
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.13.0
)
FetchContent_MakeAvailable(spdlog)

# Setup doctest for testing
find_package(doctest QUIET)
if(NOT doctest_FOUND)
    # Set policy to allow deprecated FetchContent_Populate
    if(POLICY CMP0169)
        cmake_policy(SET CMP0169 OLD)
    endif()
    FetchContent_Declare(
        doctest
        GIT_REPOSITORY https://github.com/doctest/doctest.git
        GIT_TAG v2.4.11
    )
    # Get properties to check if already populated
    FetchContent_GetProperties(doctest)
    if(NOT doctest_POPULATED)
        # Populate without processing CMakeLists.txt
        FetchContent_Populate(doctest)
    endif()
endif()

# Create doctest::doctest target if it doesn't exist
if(NOT TARGET doctest::doctest)
    if(doctest_FOUND)
        # If found via find_package, it should provide the target
        # But if not, create it
        if(NOT TARGET doctest)
            add_library(doctest INTERFACE)
            if(doctest_INCLUDE_DIRS)
                target_include_directories(doctest INTERFACE ${doctest_INCLUDE_DIRS})
            endif()
        endif()
        if(NOT TARGET doctest::doctest)
            add_library(doctest::doctest ALIAS doctest)
        endif()
    else()
        # Using FetchContent - create the target
        if(NOT TARGET doctest)
            add_library(doctest INTERFACE)
            if(DEFINED doctest_SOURCE_DIR)
                target_include_directories(doctest INTERFACE ${doctest_SOURCE_DIR})
            else()
                message(FATAL_ERROR "doctest_SOURCE_DIR is not defined. FetchContent may have failed.")
            endif()
        endif()
        add_library(doctest::doctest ALIAS doctest)
    endif()
endif()

add_library(Ribble SHARED
        include/ribble.h
        source/ribble.cpp
        include/ribble/window/window.h
        include/ribble/scene/component/audio_listener.h
        include/ribble/scene/component/audio_source.h
        include/ribble/scene/component/camera.h
        include/ribble/scene/component/component.h
        include/ribble/scene/component/light.h
        include/ribble/scene/component/mesh_renderer.h
        include/ribble/scene/component/script.h
        include/ribble/scene/component/transform.h
        include/ribble/scene/component_manager.h
        include/ribble/scene/node.h
        include/ribble/scene/scene.h
        include/ribble/scene/scene_manager.h
        include/ribble/scene/system.h
        include/ribble/scene/system_manager.h
        include/ribble/scene/component/ai/ai_controller.h
        include/ribble/scene/component/ai/behavior_tree.h
        include/ribble/scene/component/ai/state_machine.h
        include/ribble/scene/component/animation/animation_controller.h
        include/ribble/scene/component/animation/animator.h
        include/ribble/scene/component/animation/skeleton.h
        include/ribble/scene/component/cloth/cloth_renderer.h
        include/ribble/scene/component/cloth/cloth_simulation.h
        include/ribble/scene/component/effects/bloom.h
        include/ribble/scene/component/effects/motion_blur.h
        include/ribble/scene/component/effects/post_processing.h
        include/ribble/scene/component/effects/ssao.h
        include/ribble/scene/component/effects/volumetric_fog.h
        include/ribble/scene/component/input/input_handler.h
        include/ribble/scene/component/navigation/nav_mesh_agent.h
        include/ribble/scene/component/navigation/nav_mesh_obstacle.h
        include/ribble/scene/component/navigation/off_mesh_link.h
        include/ribble/scene/component/network/network_identity.h
        include/ribble/scene/component/network/network_transform.h
        include/ribble/scene/component/physics/box_collider.h
        include/ribble/scene/component/physics/capsule_collider.h
        include/ribble/scene/component/physics/character_controller.h
        include/ribble/scene/component/physics/collider.h
        include/ribble/scene/component/physics/joint.h
        include/ribble/scene/component/physics/mesh_collider.h
        include/ribble/scene/component/physics/physics_material.h
        include/ribble/scene/component/physics/rigidbody.h
        include/ribble/scene/component/physics/sphere_collider.h
        include/ribble/scene/component/physics/trigger.h
        include/ribble/scene/component/rendering/light_probe.h
        include/ribble/scene/component/rendering/line_renderer.h
        include/ribble/scene/component/rendering/lod_group.h
        include/ribble/scene/component/rendering/occlusion_culling.h
        include/ribble/scene/component/rendering/particle_system.h
        include/ribble/scene/component/rendering/reflection_probe.h
        include/ribble/scene/component/rendering/skybox.h
        include/ribble/scene/component/rendering/sprite_renderer.h
        include/ribble/scene/component/rendering/trail_renderer.h
        include/ribble/scene/component/terrain/terrain.h
        include/ribble/scene/component/terrain/terrain_collider.h
        include/ribble/scene/component/terrain/water.h
        include/ribble/scene/component/terrain/wind_zone.h
        include/ribble/scene/component/time/time_manager.h
        include/ribble/scene/component/ui/button.h
        include/ribble/scene/component/ui/canvas.h
        include/ribble/scene/component/ui/image.h
        include/ribble/scene/component/ui/input_field.h
        include/ribble/scene/component/ui/panel.h
        include/ribble/scene/component/ui/slider.h
        include/ribble/scene/component/ui/text.h
        include/ribble/scene/component/ui/ui_element.h
        include/ribble/scene/component/vehicle/buoyancy.h
        include/ribble/scene/component/vehicle/vehicle_controller.h
        include/ribble/scene/component/vehicle/wheel_collider.h
        include/ribble/asset/asset_manager.h
        include/ribble/asset/audio_loader.h
        include/ribble/asset/model_loader.h
        include/ribble/asset/shader_loader.h
        include/ribble/asset/texture_loader.h
        include/ribble/audio/audio_clip.h
        include/ribble/audio/audio_manager.h
        include/ribble/audio/audio_source_high_level.h
        include/ribble/core/config.h
        include/ribble/core/engine.h
        include/ribble/core/fail.h
        include/ribble/core/event.h
        include/ribble/core/logger.h
        include/ribble/core/math.h
        include/ribble/core/types.h
        include/ribble/input/gamepad.h
        include/ribble/input/input_manager.h
        include/ribble/input/input_types.h
        include/ribble/input/key_code.h
        include/ribble/input/mouse_button.h
        include/ribble/render/camera/camera_system.h
        include/ribble/render/camera/frustum.h
        include/ribble/render/camera/projection.h
        include/ribble/render/lighting/light_types.h
        include/ribble/render/lighting/lighting_system.h
        include/ribble/render/lighting/shadow_map.h
        include/ribble/render/lighting/shadow_mapping.h
        include/ribble/render/batch_renderer.h
        include/ribble/render/buffer.h
        include/ribble/render/framebuffer.h
        include/ribble/render/material.h
        include/ribble/render/material_builder.h
        include/ribble/render/mesh.h
        include/ribble/render/parsed_shader_source.h
        include/ribble/render/render_command.h
        include/ribble/render/render_command_queue.h
        include/ribble/render/render_pass.h
        include/ribble/render/render_pass_builder.h
        include/ribble/render/render_resource_manager.h
        include/ribble/render/render_state.h
        include/ribble/render/renderer.h
        include/ribble/render/shader.h
        include/ribble/render/shader_library.h
        include/ribble/render/shader_parser.h
        include/ribble/render/texture.h
        include/ribble/render/vertex_array.h
        include/ribble/render/vertex_attribute.h
        source/asset/asset_manager.cpp
        source/asset/audio_loader.cpp
        source/asset/model_loader.cpp
        source/asset/shader_loader.cpp
        source/asset/texture_loader.cpp
        source/audio/audio_clip.cpp
        source/audio/audio_manager.cpp
        source/audio/audio_source_high_level.cpp
        source/core/config.cpp
        source/core/engine.cpp
        source/core/fail.cpp
        source/core/event.cpp
        source/core/logger.cpp
        source/input/gamepad.cpp
        source/input/input_manager.cpp
        source/render/camera/camera_system.cpp
        source/render/camera/frustum.cpp
        source/render/camera/projection.cpp
        source/render/lighting/lighting_system.cpp
        source/render/lighting/shadow_map.cpp
        source/render/lighting/shadow_mapping.cpp
        source/render/batch_renderer.cpp
        source/render/buffer.cpp
        source/render/framebuffer.cpp
        source/render/material.cpp
        source/render/material_builder.cpp
        source/render/mesh.cpp
        source/render/render_command.cpp
        source/render/render_command_queue.cpp
        source/render/render_pass.cpp
        source/render/render_pass_builder.cpp
        source/render/render_resource_manager.cpp
        source/render/render_state.cpp
        source/render/renderer.cpp
        source/render/shader.cpp
        source/render/shader_library.cpp
        source/render/shader_parser.cpp
        source/render/texture.cpp
        source/render/vertex_array.cpp
        source/scene/component/ai/ai_controller.cpp
        source/scene/component/ai/behavior_tree.cpp
        source/scene/component/ai/state_machine.cpp
        source/scene/component/animation/animation_controller.cpp
        source/scene/component/animation/animator.cpp
        source/scene/component/animation/skeleton.cpp
        source/scene/component/cloth/cloth_renderer.cpp
        source/scene/component/cloth/cloth_simulation.cpp
        source/scene/component/effects/bloom.cpp
        source/scene/component/effects/motion_blur.cpp
        source/scene/component/effects/post_processing.cpp
        source/scene/component/effects/ssao.cpp
        source/scene/component/effects/volumetric_fog.cpp
        source/scene/component/input/input_handler.cpp
        source/scene/component/navigation/nav_mesh_agent.cpp
        source/scene/component/navigation/nav_mesh_obstacle.cpp
        source/scene/component/navigation/off_mesh_link.cpp
        source/scene/component/network/network_identity.cpp
        source/scene/component/network/network_transform.cpp
        source/scene/component/physics/box_collider.cpp
        source/scene/component/physics/capsule_collider.cpp
        source/scene/component/physics/character_controller.cpp
        source/scene/component/physics/collider.cpp
        source/scene/component/physics/joint.cpp
        source/scene/component/physics/mesh_collider.cpp
        source/scene/component/physics/physics_material.cpp
        source/scene/component/physics/rigidbody.cpp
        source/scene/component/physics/sphere_collider.cpp
        source/scene/component/physics/trigger.cpp
        source/scene/component/rendering/light_probe.cpp
        source/scene/component/rendering/line_renderer.cpp
        source/scene/component/rendering/lod_group.cpp
        source/scene/component/rendering/occlusion_culling.cpp
        source/scene/component/rendering/particle_system.cpp
        source/scene/component/rendering/reflection_probe.cpp
        source/scene/component/rendering/skybox.cpp
        source/scene/component/rendering/sprite_renderer.cpp
        source/scene/component/rendering/trail_renderer.cpp
        source/scene/component/terrain/terrain.cpp
        source/scene/component/terrain/terrain_collider.cpp
        source/scene/component/terrain/water.cpp
        source/scene/component/terrain/wind_zone.cpp
        source/scene/component/time/time_manager.cpp
        source/scene/component/ui/button.cpp
        source/scene/component/ui/canvas.cpp
        source/scene/component/ui/image.cpp
        source/scene/component/ui/input_field.cpp
        source/scene/component/ui/panel.cpp
        source/scene/component/ui/slider.cpp
        source/scene/component/ui/text.cpp
        source/scene/component/ui/ui_element.cpp
        source/scene/component/vehicle/buoyancy.cpp
        source/scene/component/vehicle/vehicle_controller.cpp
        source/scene/component/vehicle/wheel_collider.cpp
        source/scene/component/audio_listener.cpp
        source/scene/component/audio_source.cpp
        source/scene/component/camera.cpp
        source/scene/component/component.cpp
        source/scene/component/light.cpp
        source/scene/component/mesh_renderer.cpp
        source/scene/component/script.cpp
        source/scene/component/transform.cpp
        source/scene/component_manager.cpp
        source/scene/node.cpp
        source/scene/scene.cpp
        source/scene/scene_manager.cpp
        source/scene/system.cpp
        source/scene/system_manager.cpp
        source/window/window.cpp
)

target_include_directories(Ribble PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(Ribble PRIVATE
)

target_link_libraries(Ribble PUBLIC spdlog::spdlog)

if(UNIX)
    target_link_libraries(Ribble PUBLIC dl)
endif()



# Examples
add_subdirectory(examples/test_example)

# Tests
add_subdirectory(tests/backend)
add_subdirectory(tests/integration)
add_subdirectory(tests/unit)

# Backend
add_subdirectory(backend/common)
add_subdirectory(backend/audio/miniaudio)
add_subdirectory(backend/render/dx12)
add_subdirectory(backend/render/metal)
add_subdirectory(backend/render/opengl)
add_subdirectory(backend/render/vulkan)

add_subdirectory(backend/window/cocoa)
add_subdirectory(backend/window/glfw)
add_subdirectory(backend/window/sdl3)
add_subdirectory(backend/window/win32)
add_subdirectory(backend/window/x11)


# Editor
add_subdirectory(editor)
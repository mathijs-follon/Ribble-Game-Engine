# miniaudio audio backend module

# miniaudio is header-only, so we just need to include it
# Assuming miniaudio is available as a submodule or external dependency
# If not, you may need to download it or use FetchContent

add_library(ribble_backend_audio_miniaudio STATIC
    miniaudio_backend.cpp
    miniaudio_backend.h
)

target_include_directories(ribble_backend_audio_miniaudio PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
        ../../../include/ribble/backend
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../lib/miniaudio
)

target_link_libraries(ribble_backend_audio_miniaudio PUBLIC
    ribble_backend_common
)

target_compile_features(ribble_backend_audio_miniaudio PUBLIC cxx_std_23)

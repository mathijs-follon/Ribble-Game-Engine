# Wayland window backend module

if(UNIX AND NOT APPLE)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(WAYLAND_CLIENT REQUIRED wayland-client)
    pkg_check_modules(WAYLAND_PROTOCOLS wayland-protocols)
    pkg_check_modules(XKBCOMMON REQUIRED xkbcommon)

    if(NOT WAYLAND_PROTOCOLS_FOUND)
        message(FATAL_ERROR "wayland-protocols not found - required for xdg-shell")
    endif()

    # Generate xdg-shell protocol
    find_program(WAYLAND_SCANNER wayland-scanner REQUIRED)
    set(XDG_SHELL_XML "${WAYLAND_PROTOCOLS_PREFIX}/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml")
    if(NOT EXISTS "${XDG_SHELL_XML}")
        set(XDG_SHELL_XML "/usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml")
    endif()
    if(NOT EXISTS "${XDG_SHELL_XML}")
        message(FATAL_ERROR "xdg-shell.xml not found. Install wayland-protocols.")
    endif()

    set(XDG_SHELL_CLIENT_HEADER "${CMAKE_CURRENT_BINARY_DIR}/xdg-shell-client-protocol.h")
    set(XDG_SHELL_PRIVATE_CODE "${CMAKE_CURRENT_BINARY_DIR}/xdg-shell-protocol.c")
    add_custom_command(OUTPUT "${XDG_SHELL_CLIENT_HEADER}"
        COMMAND ${WAYLAND_SCANNER} client-header ${XDG_SHELL_XML} ${XDG_SHELL_CLIENT_HEADER}
        DEPENDS ${XDG_SHELL_XML})
    add_custom_command(OUTPUT "${XDG_SHELL_PRIVATE_CODE}"
        COMMAND ${WAYLAND_SCANNER} private-code ${XDG_SHELL_XML} ${XDG_SHELL_PRIVATE_CODE}
        DEPENDS ${XDG_SHELL_XML})

    add_library(ribble_backend_window_wayland STATIC
        wayland_backend.cpp
        wayland_backend.h
        ${XDG_SHELL_CLIENT_HEADER}
        ${XDG_SHELL_PRIVATE_CODE}
    )

    target_include_directories(ribble_backend_window_wayland PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
        ../../../include/
        ${WAYLAND_CLIENT_INCLUDE_DIRS}
        ${XKBCOMMON_INCLUDE_DIRS}
    )

    target_link_libraries(ribble_backend_window_wayland PUBLIC
        ${WAYLAND_CLIENT_LIBRARIES}
        ${XKBCOMMON_LIBRARIES}
        ribble_backend_common
    )

    target_compile_features(ribble_backend_window_wayland PUBLIC cxx_std_23)

    set_target_properties(ribble_backend_window_wayland PROPERTIES
        POSITION_INDEPENDENT_CODE ON
    )
else()
    add_library(ribble_backend_window_wayland INTERFACE)
    message(STATUS "Wayland window backend skipped (Linux/Unix only)")
endif()

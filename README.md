# Ribble

A cross-platform game engine written in C++23.

## Building

```bash
# Configure
cmake -B build -S .

# Build all targets
cmake --build build

# Build specific target
cmake --build build --target Ribble

# List all available targets with a weird AI generated command
cmake --build build --target help | grep -E "^[A-Za-z_]+:" | cut -d: -f1 | sort -u
```

## Formatting

```bash
# Format all project files
clang-format -i $(git ls-files '*.cpp' '*.h')
```


## Requirements

- CMake 4.2.3 or later
- C++23 compatible compiler
- Platform-specific dependencies (SDL3, OpenGL, Vulkan, etc.)

## Testing

Tests use the doctest framework and are built by default:

```bash
cmake --build build --target ribble_unit_tests
./build/tests/unit/ribble_unit_tests
```

## Project Structure

- `include/` - Public headers
- `source/` - Implementation files
- `backend/` - Platform-specific backends (render, window, audio)
- `editor/` - Editor application
- `examples/` - Example projects
- `tests/` - Test suite


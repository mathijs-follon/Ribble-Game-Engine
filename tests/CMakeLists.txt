# Test suite for Ribble Engine
# Note: doctest is configured in the root CMakeLists.txt

# Add backend common as dependency
add_subdirectory(../include/ribble/backend)

# Unit tests
add_subdirectory(unit)

# Integration tests
add_subdirectory(integration)

# Backend tests
add_subdirectory(backend)

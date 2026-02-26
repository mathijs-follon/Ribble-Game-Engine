#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace backend {

    /// Default vertex shader SPIR-V (pos, normal, texcoord -> MVP transform)
    std::vector<uint32_t> get_default_vertex_spirv();

    /// Default fragment shader SPIR-V (outputs push constant color)
    std::vector<uint32_t> get_default_fragment_spirv();

} // namespace backend

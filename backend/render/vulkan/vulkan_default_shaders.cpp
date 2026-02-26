#include "vulkan_default_shaders.h"
#include <cstring>

#ifdef GLSLANG_VALIDATOR_FOUND
#include "default_vert_spv.h"
#include "default_frag_spv.h"
#endif

namespace backend {

    std::vector<uint32_t> get_default_vertex_spirv() {
#ifdef GLSLANG_VALIDATOR_FOUND
        std::vector<uint32_t> out;
        out.resize(default_vert_spv_len / sizeof(uint32_t));
        memcpy(out.data(), default_vert_spv, default_vert_spv_len);
        return out;
#else
        return {};
#endif
    }

    std::vector<uint32_t> get_default_fragment_spirv() {
#ifdef GLSLANG_VALIDATOR_FOUND
        std::vector<uint32_t> out;
        out.resize(default_frag_spv_len / sizeof(uint32_t));
        memcpy(out.data(), default_frag_spv, default_frag_spv_len);
        return out;
#else
        return {};
#endif
    }

} // namespace backend

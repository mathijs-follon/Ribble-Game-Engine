#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "backend_types.h"

namespace backend {
    /// Language-agnostic shader source container
    /// Contains either source code (for GLSL, HLSL, MSL) or bytecode (for SPIR-V)
    struct ShaderSource {
        ShaderLanguage language{ShaderLanguage::GLSL};
        ShaderStage stage{ShaderStage::Vertex};
        std::vector<uint8_t> data; // Source code or bytecode
        std::string entryPoint; // Entry point name (for HLSL/MSL, optional for GLSL)

        ShaderSource() = default;

        /// Create from string source (GLSL, HLSL, MSL)
        ShaderSource(ShaderLanguage lang, ShaderStage stg, const std::string &source, const std::string &entry = "") :
            language(lang), stage(stg), entryPoint(entry) {
            data.assign(reinterpret_cast<const uint8_t *>(source.data()),
                        reinterpret_cast<const uint8_t *>(source.data()) + source.size());
        }

        /// Create from bytecode (SPIR-V)
        ShaderSource(ShaderStage stg, const std::vector<uint8_t> &bytecode) :
            language(ShaderLanguage::SPIRV), stage(stg), data(bytecode) {}

        /// Get as string (for source code languages)
        [[nodiscard]] std::string as_string() const {
            return std::string(reinterpret_cast<const char *>(data.data()), data.size());
        }

        /// Check if this is bytecode (SPIR-V)
        [[nodiscard]] bool is_bytecode() const { return language == ShaderLanguage::SPIRV; }
    };
} // namespace backend

#pragma once
#include <string>
#include <unordered_map>
#include "../../backend/common/backend_types.h"
#include "ribble/core/fail.h"

namespace ribble::render {

    enum class ShaderParseFailure {
        FileNotFound = 0,
        InvalidSyntax = 1,
        MissingShaderType = 2,
        EmptyShaderSource = 3,
    };

    /// Stores parsed shader source code for multiple shader stages
    /// Used by GLSL parser to store shader stages from a single file
    struct ParsedShaderSource {
        std::unordered_map<backend::ShaderStage, std::string> shaders;
    };

} // namespace ribble::render

RIBBLE_ENUM_TO_STRING(ribble::render::ShaderParseFailure,
                      case ribble::render::ShaderParseFailure::FileNotFound : return "File Not Found";
                      case ribble::render::ShaderParseFailure::InvalidSyntax : return "Invalid Syntax";
                      case ribble::render::ShaderParseFailure::MissingShaderType : return "Missing Shader Type";
                      case ribble::render::ShaderParseFailure::EmptyShaderSource : return "Empty Shader Source";);

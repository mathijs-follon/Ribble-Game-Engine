#pragma once
#include <string>
#include "parsed_shader_source.h"

namespace ribble::render {

    /// GLSL-specific shader parser
    /// Parses GLSL files with the following syntax:
    /// @code
    /// #start Vertex
    /// // vertex shader code here
    /// #end
    /// #start Fragment
    /// // fragment shader code here
    /// #end
    /// // and so on ...
    /// @endcode
    class ShaderParser {
    public:
        /// Parse a GLSL shader file
        /// @param filepath Path to the shader file
        /// @return Parsed shader source or error
        static core::Result<ParsedShaderSource, ShaderParseFailure> parse_file(const std::string &filepath);

        /// Parse GLSL shader source string
        /// @param source Shader source code
        /// @return Parsed shader source or error
        static core::Result<ParsedShaderSource, ShaderParseFailure> parse_source(const std::string &source);

    private:
        /// Parse shader type string to ShaderStage enum
        static backend::ShaderStage parse_shader_stage(const std::string &typeStr);

        /// Convert ShaderStage to string for error messages
        static std::string shader_stage_to_string(backend::ShaderStage stage);
    };

} // namespace ribble::render

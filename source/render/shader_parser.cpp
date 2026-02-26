#include "ribble/render/shader_parser.h"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include "ribble/core/logger.h"

namespace ribble::render {

    backend::ShaderStage ShaderParser::parse_shader_stage(const std::string &typeStr) {
        std::string lower = typeStr;
        std::ranges::transform(lower, lower.begin(), ::tolower);

        if (lower == "vertex")
            return backend::ShaderStage::Vertex;
        if (lower == "fragment")
            return backend::ShaderStage::Fragment;
        if (lower == "geometry")
            return backend::ShaderStage::Geometry;
        if (lower == "compute")
            return backend::ShaderStage::Compute;
        if (lower == "tessellationcontrol" || lower == "tesscontrol")
            return backend::ShaderStage::TessellationControl;
        if (lower == "tessellationevaluation" || lower == "tesseval")
            return backend::ShaderStage::TessellationEvaluation;

        return backend::ShaderStage::Vertex; // Default
    }

    std::string ShaderParser::shader_stage_to_string(backend::ShaderStage stage) {
        switch (stage) {
            case backend::ShaderStage::Vertex:
                return "Vertex";
            case backend::ShaderStage::Fragment:
                return "Fragment";
            case backend::ShaderStage::Geometry:
                return "Geometry";
            case backend::ShaderStage::Compute:
                return "Compute";
            case backend::ShaderStage::TessellationControl:
                return "TessellationControl";
            case backend::ShaderStage::TessellationEvaluation:
                return "TessellationEvaluation";
            default:
                return "Unknown";
        }
    }

    core::Result<ParsedShaderSource, ShaderParseFailure> ShaderParser::parse_file(const std::string &filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            RIBBLE_LOG_ERROR("Failed to open shader file: {}", filepath);
            return core::Fail(RIBBLE_ERROR(ShaderParseFailure::FileNotFound, "Failed to open shader file."));
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();

        auto result = parse_source(buffer.str());
        if (!result) {
            RIBBLE_LOG_ERROR("Failed to parse shader file: {}", filepath);
        }

        return result;
    }

    core::Result<ParsedShaderSource, ShaderParseFailure> ShaderParser::parse_source(const std::string &source) {
        ParsedShaderSource result;

        std::istringstream stream(source);
        std::string line;
        std::string currentShaderSource;
        backend::ShaderStage currentStage = backend::ShaderStage::Vertex;
        bool inShader = false;
        size_t lineNumber = 0;

        while (std::getline(stream, line)) {
            lineNumber++;

            size_t start = line.find_first_not_of(" \t");
            if (start == std::string::npos) {
                if (inShader) {
                    currentShaderSource += "\n";
                }
                continue;
            }

            std::string trimmed = line.substr(start);

            if (trimmed.starts_with("#start")) {
                if (inShader) {
                    RIBBLE_LOG_ERROR("Invalid syntax: #start found while already in shader block at line {}",
                                     lineNumber);
                    return core::Fail(RIBBLE_ERROR(ShaderParseFailure::InvalidSyntax,
                                                   "Invalid syntax: #start found while already in shader block."));
                }

                const size_t typeStart = trimmed.find_first_not_of(" \t", 6);
                if (typeStart == std::string::npos) {
                    RIBBLE_LOG_ERROR("Invalid syntax: #start directive missing shader type at line {}", lineNumber);
                    return core::Fail(RIBBLE_ERROR(ShaderParseFailure::InvalidSyntax,
                                                   "Invalid syntax: #start directive missing shader type."));
                }

                std::string typeStr = trimmed.substr(typeStart);
                typeStr.erase(typeStr.find_last_not_of(" \t") + 1);

                currentStage = parse_shader_stage(typeStr);
                inShader = true;
                currentShaderSource.clear();
                continue;
            }

            if (trimmed.starts_with("#end")) {
                if (!inShader) {
                    RIBBLE_LOG_ERROR("Invalid syntax: #end found without matching #start at line {}", lineNumber);
                    return core::Fail(RIBBLE_ERROR(ShaderParseFailure::InvalidSyntax,
                                                   "Invalid syntax: #end found without matching #start."));
                }

                if (currentShaderSource.empty()) {
                    RIBBLE_LOG_ERROR("Empty shader source for {} shader at line {}",
                                     shader_stage_to_string(currentStage), lineNumber);
                    return core::Fail(RIBBLE_ERROR(ShaderParseFailure::EmptyShaderSource, "Empty shader source."));
                }

                result.shaders[currentStage] = currentShaderSource;
                inShader = false;
                currentShaderSource.clear();
                continue;
            }

            if (inShader) {
                currentShaderSource += line + "\n";
            }
        }

        if (inShader) {
            RIBBLE_LOG_ERROR("Invalid syntax: Unclosed shader block (missing #end)");
            return core::Fail(RIBBLE_ERROR(ShaderParseFailure::InvalidSyntax,
                                           "Invalid syntax: Unclosed shader block (missing #end)."));
        }

        if (result.shaders.empty()) {
            RIBBLE_LOG_ERROR("No shader types found in source");
            return core::Fail(RIBBLE_ERROR(ShaderParseFailure::MissingShaderType, "No shader types found in source."));
        }

        return core::Ok(result);
    }

} // namespace ribble::render

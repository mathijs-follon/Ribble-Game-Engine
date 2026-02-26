#pragma once

namespace ribble::util {
    // parse hex nibble (0-15)
    inline int hex_digit(char c) {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
        if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
        return -1;
    }

    static glm::vec4 rgba8_to_float(
        uint8_t r,
        uint8_t g,
        uint8_t b,
        uint8_t a = 255
    ) noexcept
    {
        constexpr float inv255 = 1.0f / 255.0f;
        return {
            r * inv255,
            g * inv255,
            b * inv255,
            a * inv255
        };
    }

}
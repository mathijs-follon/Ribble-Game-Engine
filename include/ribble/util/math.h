#pragma once
#include <cstdint>

namespace ribble::util {
    inline float clamp01(float v) noexcept { return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v); }

    inline uint8_t float_to_u8(float v) noexcept {
        // clamp to [0,1] then map to [0,255]
        float c = clamp01(v);
        return static_cast<uint8_t>(c * 255.0f + 0.5f);
    }

    inline float u8_to_float(uint8_t v) noexcept { return static_cast<float>(v) / 255.0f; }


} // namespace ribble::util

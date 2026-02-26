#pragma once
#include <iomanip>
#include <ios>
#include <iosfwd>
#include <string>
#include <glm/glm.hpp>

#include "ribble/util/color.h"
#include "ribble/util/math.h"

namespace ribble::render {
    class Color {
    protected:
        glm::vec4 m_rgba{0.0f, 0.0f, 0.0f, 1.0f};

        explicit Color(glm::vec4 rgba) noexcept : m_rgba(rgba) {
            m_rgba.x = util::clamp01(m_rgba.x);
            m_rgba.y = util::clamp01(m_rgba.y);
            m_rgba.z = util::clamp01(m_rgba.z);
            m_rgba.w = util::clamp01(m_rgba.w);
        }

    public:
        Color() = default;
        virtual ~Color() = default;

        glm::vec4 to_RGBA() const noexcept { return m_rgba; }
        float r() const noexcept { return m_rgba.r; }
        float g() const noexcept { return m_rgba.g; }
        float b() const noexcept { return m_rgba.b; }
        float a() const noexcept { return m_rgba.a; }

        void set_RGBA(const glm::vec4 &rgba) noexcept {
            m_rgba = glm::clamp(rgba, glm::vec4(0.0f), glm::vec4(1.0f));
        }

        uint32_t to_RGBA8() const noexcept {
            uint32_t R = util::float_to_u8(m_rgba.r);
            uint32_t G = util::float_to_u8(m_rgba.g);
            uint32_t B = util::float_to_u8(m_rgba.b);
            uint32_t A = util::float_to_u8(m_rgba.a);
            return (R << 24) | (G << 16) | (B << 8) | A;
        }

        std::string to_hex_string(bool includeAlpha = false) const {
            std::ostringstream ss;
            ss << '#'
               << std::hex << std::setfill('0') << std::uppercase
               << std::setw(2) << static_cast<int>(util::float_to_u8(m_rgba.r))
               << std::setw(2) << static_cast<int>(util::float_to_u8(m_rgba.g))
               << std::setw(2) << static_cast<int>(util::float_to_u8(m_rgba.b));
            if (includeAlpha)
                ss << std::setw(2) << static_cast<int>(util::float_to_u8(m_rgba.a));
            return ss.str();
        }

        uint16_t to_RGB565() const noexcept {
            uint16_t r5 = static_cast<uint16_t>(util::float_to_u8(m_rgba.r) >> 3) & 0x1F;
            uint16_t g6 = static_cast<uint16_t>(util::float_to_u8(m_rgba.g) >> 2) & 0x3F;
            uint16_t b5 = static_cast<uint16_t>(util::float_to_u8(m_rgba.b) >> 3) & 0x1F;
            return static_cast<uint16_t>((r5 << 11) | (g6 << 5) | b5);
        }

        // Convert to grayscale luminance (standard Rec. 709 luminance)
        float to_gray_luminance() const noexcept {
            return 0.2126f * m_rgba.r + 0.7152f * m_rgba.g + 0.0722f * m_rgba.b;
        }
    };

    class ColorRGBA final : public Color {
    public:
        ColorRGBA() noexcept : Color({0.0f, 0.0f, 0.0f, 1.0f}) {}
        ColorRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a) noexcept : Color(util::rgba8_to_float(r, g, b, a)) {}
        ColorRGBA(float r, float g, float b, float a = 1.0f) noexcept : Color({r, g, b, a}) {}
        explicit ColorRGBA(const glm::vec4 &rgba) noexcept : Color(rgba) {}
    };

    class ColorRGB final : public Color {
    public:
        ColorRGB() noexcept : Color({0.0f, 0.0f, 0.0f, 1.0f}) {}
        ColorRGB(uint8_t r, uint8_t g, uint8_t b) noexcept : Color(util::rgba8_to_float(r, g, b)) {}
        ColorRGB(float r, float g, float b) noexcept : Color({r, g, b, 1.0f}) {}
        explicit ColorRGB(const glm::vec3 &rgb) noexcept : Color({rgb.r, rgb.g, rgb.b, 1.0f}) {}
    };

    class ColorGrayScale final : public Color {
    public:
        // luminance in [0,1], alpha in [0,1]
        ColorGrayScale(float luminance = 0.0f, float alpha = 1.0f) noexcept
            : Color({luminance, luminance, luminance, alpha}) {}

        float luminance() const noexcept { return to_gray_luminance(); }
    };

    class ColorRGB565 final : public Color {
    public:
        explicit ColorRGB565(uint16_t packed) noexcept
            : Color(rgb565_to_rgba(packed)) {}

        ColorRGB565(float r, float g, float b) noexcept : Color({r, g, b, 1.0f}) {}

        uint16_t packed() const noexcept { return to_RGB565(); }

        static glm::vec4 rgb565_to_rgba(uint16_t packed) noexcept {
            uint32_t r5 = (packed >> 11) & 0x1F;
            uint32_t g6 = (packed >> 5) & 0x3F;
            uint32_t b5 = packed & 0x1F;
            // expand to 8-bit then to float
            uint8_t r8 = static_cast<uint8_t>((r5 << 3) | (r5 >> 2));
            uint8_t g8 = static_cast<uint8_t>((g6 << 2) | (g6 >> 4));
            uint8_t b8 = static_cast<uint8_t>((b5 << 3) | (b5 >> 2));
            return glm::vec4(util::u8_to_float(r8), util::u8_to_float(g8), util::u8_to_float(b8), 1.0f);
        }
    };

    class ColorHex final : public Color {
    public:
        // Accept many formats: "#RRGGBB", "#RRGGBBAA", "RRGGBB", "RRGGBBAA",
        // short "#RGB" or "#RGBA".
        explicit ColorHex(const std::string &hex) : Color(parse_hex(hex)) {}
        explicit ColorHex(uint32_t rgba8888) noexcept : Color(uint32_to_rgba(rgba8888)) {}

        // Create from components 0-255
        ColorHex(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) noexcept
            : Color(glm::vec4(util::u8_to_float(r), util::u8_to_float(g), util::u8_to_float(b), util::u8_to_float(a))) {}

        static glm::vec4 uint32_to_rgba(uint32_t v) noexcept {
            uint8_t r = static_cast<uint8_t>((v >> 24) & 0xFF);
            uint8_t g = static_cast<uint8_t>((v >> 16) & 0xFF);
            uint8_t b = static_cast<uint8_t>((v >> 8) & 0xFF);
            uint8_t a = static_cast<uint8_t>(v & 0xFF);
            return glm::vec4(util::u8_to_float(r), util::u8_to_float(g), util::u8_to_float(b), util::u8_to_float(a));
        }

    private:
        static glm::vec4 parse_hex(const std::string &s) {
            std::string in = s;
            std::erase_if(in, ::isspace);
            if (!in.empty() && in[0] == '#') in.erase(in.begin());

            if (in.size() == 3 || in.size() == 4) {
                int r = util::hex_digit(in[0]);
                int g = util::hex_digit(in[1]);
                int b = util::hex_digit(in[2]);
                int a = (in.size() == 4) ? util::hex_digit(in[3]) : 15;
                if (r < 0 || g < 0 || b < 0 || a < 0) throw std::invalid_argument("Invalid hex string");
                uint8_t R = static_cast<uint8_t>((r << 4) | r);
                uint8_t G = static_cast<uint8_t>((g << 4) | g);
                uint8_t B = static_cast<uint8_t>((b << 4) | b);
                uint8_t A = static_cast<uint8_t>((a << 4) | a);
                return glm::vec4(util::u8_to_float(R), util::u8_to_float(G), util::u8_to_float(B), util::u8_to_float(A));
            }
            if (in.size() == 6 || in.size() == 8) {
                auto parse_byte = [&](int idx)->uint8_t {
                    int hi = util::hex_digit(in[idx]);
                    int lo = util::hex_digit(in[idx + 1]);
                    if (hi < 0 || lo < 0) throw std::invalid_argument("Invalid hex string");
                    return static_cast<uint8_t>((hi << 4) | lo);
                };
                uint8_t R = parse_byte(0);
                uint8_t G = parse_byte(2);
                uint8_t B = parse_byte(4);
                uint8_t A = (in.size() == 8) ? parse_byte(6) : 255;
                return glm::vec4(util::u8_to_float(R), util::u8_to_float(G), util::u8_to_float(B), util::u8_to_float(A));
            }

            throw std::invalid_argument("Hex string must be 3,4,6 or 8 hex digits (with optional leading '#')");
        }
    };

    class ColorHSL final : public Color {
    public:
        // h in degrees [0,360), s and l in [0,1], alpha in [0,1]
        ColorHSL(float h, float s, float l, float alpha = 1.0f) noexcept
            : Color(hsl_to_rgba(h, s, l, alpha)) {}

        float hue() const noexcept {
            float h, s, l;
            rgba_to_hsl(m_rgba, h, s, l);
            return h;
        }
        float saturation() const noexcept {
            float h, s, l;
            rgba_to_hsl(m_rgba, h, s, l);
            return s;
        }
        float lightness() const noexcept {
            float h, s, l;
            rgba_to_hsl(m_rgba, h, s, l);
            return l;
        }
        float alpha() const noexcept { return m_rgba.a; }

    private:
        static glm::vec4 hsl_to_rgba(float h_deg, float s, float l, float alpha = 1.0f) noexcept {
            // Normalize hue to [0,360)
            float h = std::fmod(h_deg, 360.0f);
            if (h < 0.0f) h += 360.0f;
            s = util::clamp01(s);
            l = util::clamp01(l);
            alpha = util::clamp01(alpha);

            if (s == 0.0f) {
                // achromatic
                return glm::vec4(l, l, l, alpha);
            }

            float c = (1.0f - std::fabs(2.0f * l - 1.0f)) * s;
            float h_prime = h / 60.0f;
            float x = c * (1.0f - std::fabs(std::fmod(h_prime, 2.0f) - 1.0f));
            float r1 = 0.0f, g1 = 0.0f, b1 = 0.0f;

            if (h_prime >= 0.0f && h_prime < 1.0f) { r1 = c; g1 = x; b1 = 0.0f; }
            else if (h_prime < 2.0f) { r1 = x; g1 = c; b1 = 0.0f; }
            else if (h_prime < 3.0f) { r1 = 0.0f; g1 = c; b1 = x; }
            else if (h_prime < 4.0f) { r1 = 0.0f; g1 = x; b1 = c; }
            else if (h_prime < 5.0f) { r1 = x; g1 = 0.0f; b1 = c; }
            else { r1 = c; g1 = 0.0f; b1 = x; }

            float m = l - c / 2.0f;
            return glm::vec4(r1 + m, g1 + m, b1 + m, alpha);
        }

        static void rgba_to_hsl(const glm::vec4 &rgba, float &out_h, float &out_s, float &out_l) noexcept {
            float r = rgba.r, g = rgba.g, b = rgba.b;
            float maxc = std::max(r, std::max(g, b));
            float minc = std::min(r, std::min(g, b));
            float delta = maxc - minc;
            out_l = (maxc + minc) / 2.0f;

            if (delta == 0.0f) {
                out_h = 0.0f;
                out_s = 0.0f;
                return;
            }

            if (out_l < 0.5f)
                out_s = delta / (maxc + minc);
            else
                out_s = delta / (2.0f - maxc - minc);

            if (maxc == r) {
                out_h = (g - b) / delta;
            } else if (maxc == g) {
                out_h = 2.0f + (b - r) / delta;
            } else {
                out_h = 4.0f + (r - g) / delta;
            }
            out_h *= 60.0f;
            if (out_h < 0.0f) out_h += 360.0f;
        }
    };

}

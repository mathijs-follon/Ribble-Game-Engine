#pragma once
#include <string>

#define RIBBLE_ENUM_TO_STRING(EnumType, CASES)                                                                         \
    template<>                                                                                                         \
    struct ribble::util::EnumToString<EnumType> {                                                                      \
        static const char *ToString(EnumType v) noexcept {                                                             \
            switch (v) {                                                                                               \
                CASES                                                                                                  \
                default:                                                                                               \
                    return nullptr;                                                                                    \
            }                                                                                                          \
        }                                                                                                              \
    }


namespace ribble::util {
    template<typename T>
    struct EnumToString {
        static constexpr const char *ToString(T) noexcept { return nullptr; }
    };

    template<typename T>
    std::string EnumValueToString(T v) {
        static_assert(std::is_enum_v<T>, "EnumValueToString requires enum type T");

        if (const char *name = EnumToString<T>::ToString(v)) {
            return {name};
        }
        using UT = std::underlying_type_t<T>;
        return std::to_string(static_cast<UT>(v));
    }

} // namespace ribble::util

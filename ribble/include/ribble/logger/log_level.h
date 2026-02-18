#ifndef RIBBLE_LOG_LEVEL_H
#define RIBBLE_LOG_LEVEL_H

#include <cstdint>

namespace ribble::logger {
    enum class LogLevel : uint8_t {
        Debug = 0b1,
        Info = 0b10,
        Warning = 0b100,
        Error = 0b1000,
        Count = 5
    };

    constexpr uint8_t operator|(LogLevel a, LogLevel b) {
        return static_cast<uint8_t>(a) | static_cast<uint8_t>(b);
    }
}

#endif // RIBBLE_LOG_LEVEL_H


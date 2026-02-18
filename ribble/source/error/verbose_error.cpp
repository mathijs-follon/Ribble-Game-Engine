//
// Created by Mathijs Follon on 2/17/26.
//
#include <ribble/error/verbose_error.h>

namespace ribble::error {
#ifdef RIBBLE_DEBUG
    VerboseError::VerboseError(const uint8_t failure, const std::string& message)
        : Error{failure, false}
        , m_message{message}
    {
    }

    VerboseError::VerboseError(const uint8_t failure, const char* file, const size_t line, const std::string& message)
        : Error{failure, Error::ErrorLocation{file, line}, false}
        , m_message{message}
    {
    }

    void VerboseError::Throw(const uint8_t failure, const std::string& message) {
        VerboseError error{failure, message};
        if (Error::s_callback) {
            Error::s_callback(error);
        }
    }

    void VerboseError::Throw(const uint8_t failure, const char* file, const size_t line, const std::string& message) {
        VerboseError error{failure, file, line, message};
        if (Error::s_callback) {
            Error::s_callback(error);
        }
    }
#else
    // Release mode: const char* parameters are ignored, no allocation overhead
    VerboseError::VerboseError(const uint8_t failure, [[maybe_unused]] const char* message)
        : Error{failure, false}
    {
    }

    VerboseError::VerboseError(const uint8_t failure, [[maybe_unused]] const char* file, [[maybe_unused]] const size_t line, [[maybe_unused]] const char* message)
        : Error{failure, false}
    {
    }

    void VerboseError::Throw(const uint8_t failure, [[maybe_unused]] const char* message) {
        VerboseError error{failure, message};
        if (Error::s_callback) {
            Error::s_callback(error);
        }
    }

    void VerboseError::Throw(const uint8_t failure, [[maybe_unused]] const char* file, [[maybe_unused]] const size_t line, [[maybe_unused]] const char* message) {
        VerboseError error{failure, file, line, message};
        if (Error::s_callback) {
            Error::s_callback(error);
        }
    }
#endif
} // namespace ribble::error
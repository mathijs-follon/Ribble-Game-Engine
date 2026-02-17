//
// Created by Mathijs Follon on 2/17/26.
//
#include <ribble/error/error.h>

namespace ribble::error {
    // Static callback initialization
    Error::ErrorCallback Error::s_callback{};

    Error::Error(const uint8_t failure, const bool fatal)
        : m_isFatal{fatal}
        , m_failure{failure}
        , m_location{}
    {
    }

    Error::Error(const uint8_t failure, [[maybe_unused]] const ErrorLocation &errorLocation, const bool fatal)
#ifdef RIBBLE_DEBUG
        : m_isFatal{fatal}
        , m_failure{failure}
        , m_location{errorLocation}
    {}
#else
    : Error{failure, fatal} {}
#endif

    std::optional<Error::ErrorLocation> Error::location() const {
#ifdef RIBBLE_DEBUG
        return std::optional{m_location};
#else
        return std::optional<ErrorLocation>{};
#endif
    }

    void Error::SetCallback(ErrorCallback callback) {
        s_callback = std::move(callback);
    }

    void Error::Throw(const uint8_t failure, const bool fatal) {
        Error error{failure, fatal};
        if (s_callback) {
            s_callback(error);
        }
    }

    void Error::Throw(const uint8_t failure, const ErrorLocation& errorLocation, const bool fatal) {
        Error error{failure, errorLocation, fatal};
        if (s_callback) {
            s_callback(error);
        }
    }

}

//
// Created by Mathijs Follon on 2/18/26.
//

#include <ribble/error/error.h>

#include <utility>

#include "ribble/logger/logger.h"

namespace ribble::error {
    Error::Error(std::string&& message, const Failure code, const bool isFatal, const char *fileName, const size_t fileLine)
    : m_fileName{fileName}
    , m_fileLine{fileLine}
    , m_message{std::move(message)}
    , m_isFatal{isFatal}
    , m_failure{code}
    {
        const char* file = file_name() ? file_name() : "undefined";
        if (is_fatal()) {
            GetLogger().error(
                "A fatal error with cause [", static_cast<uint8_t>(failure()) ,"] occurred at ",
                file, ':', file_line() ? std::to_string(file_line()) : "undefined"
            );
            throw std::runtime_error(
                "Fatal error [" + std::to_string(static_cast<uint8_t>(failure())) +
                "] at " + file + ":" +
                (file_line() ? std::to_string(file_line()) : "undefined")
            );
        }
        GetLogger().warning(
            "A non fatal error with cause [", static_cast<uint8_t>(failure()) ,"] occurred at ",
            file, ':', file_line() ? std::to_string(file_line()) : "undefined"
        );
    }

    inline const char * Error::file_name() const {
        return m_fileName;
    }

    inline size_t Error::file_line() const {
        return m_fileLine;
    }

    inline std::string_view Error::message() const {
        return m_message;
    }

    inline bool Error::is_fatal() const {
        return m_isFatal;
    }

    inline Failure Error::failure() const {
        return m_failure;
    }
}

//
// Created by Mathijs Follon on 2/17/26.
//

#ifndef RIBBLE_VERBOSE_ERROR_H
#define RIBBLE_VERBOSE_ERROR_H
#include <string>

#include "error.h"


namespace ribble::error {
    class VerboseError : public Error
    {
    public:
#ifdef RIBBLE_DEBUG
        VerboseError(uint8_t failure, const std::string& message);
        VerboseError(uint8_t failure, const char* file, size_t line, const std::string& message);
#else
        VerboseError(uint8_t failure, const char* message = nullptr);
        VerboseError(uint8_t failure, const char* file, size_t line, const char* message = nullptr);
#endif

        [[nodiscard]] const std::string_view message() const {
#ifdef RIBBLE_DEBUG
            return m_message;
#else
            return {};
#endif
        }

        // Static Throw() functions for VerboseError
#ifdef RIBBLE_DEBUG
        static void Throw(uint8_t failure, const std::string& message);
        static void Throw(uint8_t failure, const char* file, size_t line, const std::string& message);
#else
        static void Throw(uint8_t failure, const char* message = nullptr);
        static void Throw(uint8_t failure, const char* file, size_t line, const char* message = nullptr);
#endif

    private:
#ifdef RIBBLE_DEBUG
        std::string m_message;
#endif
    };
} // namespace ribble::error


#endif //RIBBLE_VERBOSE_ERROR_H
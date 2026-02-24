#pragma once

#include "logger.h"

#include <string>
#include <ribble/util/enum.h>
#include <utility>

// Check for std::expected support (C++23 feature)
#if defined(__cpp_lib_expected) || (__cplusplus >= 202302L)
    #include <expected>
#else
    #error "std::expected is not available. This requires C++23 with a standard library that supports std::expected. Please use a compiler with full C++23 support (GCC 13+, Clang 17+, MSVC 19.30+)."
#endif

namespace ribble::core {

    template<typename T>
    struct Failure {
        const char* file{};
        size_t line{};
        std::string message;
        T failureType;
        bool isFatal;

        [[nodiscard]] bool is_fatal() const {
            return isFatal;
        }

        template <typename... Args>
        Failure(
            T failureType,
            bool isFatal,
            const char* file,
            size_t line,
            const char* fmt,
            Args&&... args)
            :
            file(file),
            line(line),
            failureType(failureType),
            isFatal{isFatal}
        {
            if (auto logger = ribble::GetLogger()) {
                message = ribble::detail::FormatMessageWithLocation(
                    fmt, file, line, std::forward<Args>(args)...);

                std::string enumStr = ribble::util::EnumValueToString(failureType);
                message = "[" + enumStr + "] " + message;

                // Use synchronized logging to ensure deterministic ordering
                if (isFatal) {
                    ribble::detail::LogErrorSync(message);
                } else {
                    ribble::detail::LogWarningSync(message);
                }
            } else {
                std::string enumStr = ribble::util::EnumValueToString(failureType);
                message = "[" + enumStr + "] " + std::string(fmt);
            }
        }
    };

    template<typename R, typename F>
    struct Result : std::expected<R, Failure<F>> {
        using base_type = std::expected<R, Failure<F>>;
        
        using base_type::base_type;
        
        Result(R value) : base_type(std::in_place, value) {}
        
        Result(Failure<F> failure) : base_type(std::unexpected<Failure<F>>(failure)) {}
        
        static Result<R, F> Ok(R value) {
            return Result{value};
        }
        
        static Result<R, F> Fail(Failure<F> failure) {
            return Result{failure};
        }
    };

    template<typename R>
    struct result_ok_builder {
        R value;
        
        template<typename F>
        operator Result<R, F>() const {
            return Result<R, F>{value};
        }
    };

    template<typename F>
    struct result_fail_builder {
        Failure<F> failure;
        
        template<typename R>
        operator Result<R, F>() const {
            return Result<R, F>{failure};
        }
    };

    template<typename R>
    result_ok_builder<R> Ok(R value) {
        return result_ok_builder<R>{value};
    }

    template<typename F>
    result_fail_builder<F> Fail(Failure<F> failure) {
        return result_fail_builder<F>{failure};
    }


#define RIBBLE_WARN(failure, fmt, ...) \
ribble::core::Failure<decltype(failure)>{failure, false, __FILE__, __LINE__, fmt, ##__VA_ARGS__}

#define RIBBLE_ERROR(failure, fmt, ...) \
ribble::core::Failure<decltype(failure)>{failure, true, __FILE__, __LINE__, fmt, ##__VA_ARGS__}

#define RIBBLE_FAILURE(failure, isFatal, fmt, ...) \
ribble::core::Failure<decltype(failure)>{failure, isFatal, __FILE__, __LINE__, fmt, ##__VA_ARGS__}
}

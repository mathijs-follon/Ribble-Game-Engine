#pragma once

#include "logger.h"

#include <string>
#include <ribble/util/enum.h>
#include <utility>
#include <variant>
#include <expected>

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

        [[nodiscard]] T code() const {
            return failureType;
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

    // General Result specialization for non-void types
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

    template<typename F>
    struct Result<void, F> {
        using base_type = std::expected<std::monostate, Failure<F>>;
        base_type m_expected;

        Result() = default;

        Result(std::monostate) : m_expected(std::in_place) {}

        Result(Failure<F> failure) : m_expected(std::unexpected<Failure<F>>(failure)) {}

        Result(base_type expected) : m_expected(std::move(expected)) {}

        [[nodiscard]] bool has_value() const { return m_expected.has_value(); }
        explicit operator bool() const { return m_expected.has_value(); }
        
        const Failure<F>& error() const { return m_expected.error(); }
        Failure<F>& error() { return m_expected.error(); }
        
        const base_type& value() const { return m_expected; }
        base_type& value() { return m_expected; }

        template<typename U>
        auto value_or(U&& default_value) const {
            return m_expected.value_or(std::forward<U>(default_value));
        }

        template<typename Func>
        auto and_then(Func&& f) const {
            return m_expected.and_then([&f](const std::monostate&) {
                return f();
            });
        }

        template<typename Func>
        auto or_else(Func&& f) const {
            return m_expected.or_else(std::forward<Func>(f));
        }

        template<typename Func>
        auto transform(Func&& f) const {
            return m_expected.transform([&f](const std::monostate&) {
                return f();
            });
        }

        template<typename Func>
        auto transform_error(Func&& f) const {
            return m_expected.transform_error([&f](const Failure<F>& err) {
                if constexpr (std::is_void_v<std::invoke_result_t<Func, Failure<F>>>) {
                    f(err);
                    return err;
                } else {
                    return f(err);
                }
            });
        }

        static Result<void, F> Ok() {
            return Result{std::monostate{}};
        }
        
        static Result<void, F> Fail(Failure<F> failure) {
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

    // Specialization for void
    template<>
    struct result_ok_builder<void> {
        template<typename F>
        operator Result<void, F>() const {
            return Result<void, F>::Ok();
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

    inline result_ok_builder<void> Ok() {
        return result_ok_builder<void>{};
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

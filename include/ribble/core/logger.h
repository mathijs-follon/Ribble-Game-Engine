#pragma once

#include <cstddef>
#include <memory>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <string>

namespace ribble {

    /**
     * @brief Initialize the logging system
     *
     * Creates logger with rotating files: latest.log, error.latest.log and otherwise <datetime>.log /
     * error.<datetime>.log
     *
     * @param logDirectory Directory where log files will be stored (default: "logs")
     * @param maxFileSize Maximum size of a log file before rotation in bytes (default: 5MB)
     * @param maxFiles Maximum number of rotated log files to keep (default: 3)
     * @return true if initialization succeeded, false otherwise
     */
    bool InitializeLogger(const std::string &logDirectory = "logs",
                          size_t maxFileSize = 5 * 1024 * 1024, // 5MB
                          size_t maxFiles = 3);

    /**
     * @brief Shutdown the logging system
     *
     * Flushes all pending log messages and cleans up loggers.
     * Should be called at application shutdown.
     */
    void ShutdownLogger();

    /**
     * @brief Get the main logger instance
     *
     * @return Shared pointer to the main logger instance, or nullptr if not initialized
     * @note The logger must be initialized with InitializeLogger() before calling this function
     */
    std::shared_ptr<spdlog::logger> GetLogger();

    /**
     * @brief Set the global log level
     *
     * @param level Minimum log level to output
     */
    void SetLogLevel(spdlog::level::level_enum level);

    /**
     * @brief Get the current global log level
     *
     * @return Current log level
     */
    spdlog::level::level_enum GetLogLevel();

    // Internal synchronized logging functions for deterministic ordering
    namespace detail {
        void LogDebugSync(const std::string &message);
        void LogInfoSync(const std::string &message);
        void LogWarningSync(const std::string &message);
        void LogErrorSync(const std::string &message);
    } // namespace detail

    // Internal helper functions for formatting
    namespace detail {
        template<typename FormatString, typename... Args>
        inline std::string FormatMessage(FormatString &&fmt, Args &&...args) {
            return spdlog::fmt_lib::format(fmt::runtime(std::forward<FormatString>(fmt)), std::forward<Args>(args)...);
        }

        template<typename FormatString, typename... Args>
        inline std::string FormatMessageWithLocation(FormatString &&fmt, const char *file, size_t line,
                                                     Args &&...args) {
            std::string formatted =
                    spdlog::fmt_lib::format(fmt::runtime(std::forward<FormatString>(fmt)), std::forward<Args>(args)...);
            // Extract just the filename from the full path
            const char *filename = file;
            const char *lastSlash = file;
            while (*file) {
                if (*file == '/' || *file == '\\') {
                    lastSlash = file + 1;
                }
                ++file;
            }
            return spdlog::fmt_lib::format("[{}:{}] {}", lastSlash, line, formatted);
        }
    } // namespace detail

} // namespace ribble

// Logging macros
// Debug, Warning, and Error include __FILE__ and __LINE__
// Info does not include file/line information

#ifdef RIBBLE_DEBUG
#define RIBBLE_LOG_DEBUG(fmt, ...)                                                                                     \
    do {                                                                                                               \
        ribble::detail::LogDebugSync(                                                                                  \
                ribble::detail::FormatMessageWithLocation(fmt, __FILE__, __LINE__, ##__VA_ARGS__));                    \
    } while (0)

#define RIBBLE_LOG_INFO(fmt, ...)                                                                                      \
    do {                                                                                                               \
        ribble::detail::LogInfoSync(ribble::detail::FormatMessage(fmt, ##__VA_ARGS__));                                \
    } while (0)

#define RIBBLE_LOG_WARNING(fmt, ...)                                                                                   \
    do {                                                                                                               \
        ribble::detail::LogWarningSync(                                                                                \
                ribble::detail::FormatMessageWithLocation(fmt, __FILE__, __LINE__, ##__VA_ARGS__));                    \
    } while (0)

#define RIBBLE_LOG_ERROR(fmt, ...)                                                                                     \
    do {                                                                                                               \
        ribble::detail::LogErrorSync(                                                                                  \
                ribble::detail::FormatMessageWithLocation(fmt, __FILE__, __LINE__, ##__VA_ARGS__));                    \
    } while (0)
#else
  // Release build: only Info and Error are available
#define RIBBLE_LOG_DEBUG(fmt, ...) ((void) 0)

#define RIBBLE_LOG_INFO(fmt, ...)                                                                                      \
    do {                                                                                                               \
        if (auto logger = ribble::GetLogger()) {                                                                       \
            logger->info(ribble::detail::FormatMessage(fmt, ##__VA_ARGS__));                                           \
        }                                                                                                              \
    } while (0)

#define RIBBLE_LOG_WARNING(fmt, ...) ((void) 0)

#define RIBBLE_LOG_ERROR(fmt, ...)                                                                                     \
    do {                                                                                                               \
        ribble::detail::LogErrorSync(                                                                                  \
                ribble::detail::FormatMessageWithLocation(fmt, __FILE__, __LINE__, ##__VA_ARGS__));                    \
    } while (0)
#endif

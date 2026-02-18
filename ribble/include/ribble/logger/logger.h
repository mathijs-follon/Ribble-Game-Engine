#ifndef RIBBLE_LOGGER_H
#define RIBBLE_LOGGER_H

#include <ribble/logger/log_level.h>
#include <ribble/logger/log_target.h>

#include <filesystem>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#define RIBBLE_FILE_LOCATION \
ribble::error::Error::ErrorLocation{__FILE__, __LINE__}

namespace ribble {
    namespace logger {
        class Logger {
        public:
            explicit Logger(
                const std::string& logDirectory = "logs",
                size_t maxFileSize = 5 * 1024 * 1024,
                size_t maxFiles = 3,
                const std::string& loggerName = "ribble"
            );
            ~Logger();

            void add_target(LogTarget target, uint8_t levelMask);

            void add_console_target(uint8_t levelMask);

            void add_file_target(const std::string& filename);

            void log(LogLevel level, const std::string& message);

            template<typename... Args>
            void debug(Args&& ...args) {
#ifdef RIBBLE_DEBUG
                log(LogLevel::Debug, format(std::forward<Args>(args)...));
#endif
            }

            template<typename... Args>
            void info(Args&& ...args) {
                log(LogLevel::Info, format(std::forward<Args>(args)...));
            }

            template<typename... Args>
            void warning(Args&& ...args) {
                log(LogLevel::Warning, format(std::forward<Args>(args)...));
            }

            template<typename... Args>
            void error(Args&& ...args) {
                log(LogLevel::Error, format(std::forward<Args>(args)...));
            }

            void flush() const;

            [[nodiscard]] const std::filesystem::path& log_dir() const { return m_logDir; }

            [[nodiscard]] const std::string& name() const { return m_loggerName; }

            void set_name(const std::string& name) { m_loggerName = name; }

        private:
            template<typename... Args>
            std::string format(Args&& ...args) {
                std::stringstream ss;
                (ss << ... << args);
                return ss.str();
            }

            std::string format_message(LogLevel level, const std::string& message, bool useColors = false) const;

            [[nodiscard]] std::string_view level_name(LogLevel level) const;

            [[nodiscard]] std::string_view level_color(LogLevel level) const;

            [[nodiscard]] static constexpr std::string_view color_reset() { return "\033[0m"; }

            void rotate_log_file(LogTarget& target);

            [[nodiscard]] std::string timestamp() const;

            struct TargetEntry {
                std::shared_ptr<LogTarget> target;
                uint8_t levelMask;
            };
            
            std::vector<TargetEntry> m_targets;
            std::filesystem::path m_logDir;
            size_t m_maxFileSize;
            size_t m_maxFiles;
            std::string m_loggerName;
            mutable std::mutex m_logMutex;
        };
    }

    logger::Logger& GetLogger(
        const std::string& logDirectory = "logs",
        size_t maxFileSize = 5 * 1024 * 1024,
        size_t maxFiles = 3,
        const std::string& loggerName = "ribble"
    );
}

#endif // RIBBLE_LOGGER_H

#include <ribble/logger/logger.h>

#include <chrono>
#include <ctime>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>

namespace ribble {
    std::shared_ptr<logger::Logger> g_ribbleLogger = nullptr;

    logger::Logger& GetLogger(
        const std::string& logDirectory,
        size_t maxFileSize,
        size_t maxFiles,
        const std::string& loggerName
    ) {
        static std::mutex initMutex;
        std::lock_guard lock(initMutex);
        if (g_ribbleLogger == nullptr) {
            g_ribbleLogger = std::make_shared<logger::Logger>(
                logDirectory,
                maxFileSize,
                maxFiles,
                loggerName
            );
        }
        return *g_ribbleLogger;
    }

    namespace logger {
        Logger::Logger(const std::string& logDirectory, size_t maxFileSize, size_t maxFiles, const std::string& loggerName)
            : m_logDir(logDirectory)
            , m_maxFileSize(maxFileSize)
            , m_maxFiles(maxFiles)
            , m_loggerName(loggerName)
        {
            std::filesystem::create_directories(m_logDir);

            add_console_target(0xFF);

            add_file_target(loggerName);
        }

        Logger::~Logger() {
            flush();
        }

        void Logger::add_target(LogTarget target, uint8_t levelMask) {
            std::lock_guard<std::mutex> lock(m_logMutex);
            m_targets.push_back({std::make_shared<LogTarget>(std::move(target)), levelMask});
        }

        void Logger::add_console_target(uint8_t levelMask) {
            LogTarget consoleTarget = LogTarget::Stream(std::cout);
            add_target(std::move(consoleTarget), levelMask);
        }

        void Logger::add_file_target(const std::string& filename) {
            const std::filesystem::path filePath = m_logDir / (filename + ".log");
            LogTarget fileTarget = LogTarget::File(filePath);
            add_target(std::move(fileTarget), 0xFF);
        }

        void Logger::log(LogLevel level, const std::string& message) {
            const auto levelIndex = static_cast<size_t>(level);
            if (levelIndex >= static_cast<size_t>(LogLevel::Count)) {
                return;
            }

            const std::string formattedWithColors = format_message(level, message, true);
            const std::string formattedWithoutColors = format_message(level, message, false);

            std::lock_guard lock(m_logMutex);
            
            for (auto&[lt, levelMask] : m_targets) {
                if (uint8_t levelBit = 1 << levelIndex; !(levelMask & levelBit)) {
                    continue;
                }
                
                auto& target = *lt;

                const bool useColors = !target.is_file();
                const std::string& formatted = useColors ? formattedWithColors : formattedWithoutColors;
                
                if (target.is_file()) {
                    rotate_log_file(target);
                }
                
                target.write(formatted);
            }
        }

        void Logger::flush() const {
            std::lock_guard lock(m_logMutex);
            for (const auto&[target, levelMask] : m_targets) {
                target->flush();
            }
        }

        std::string Logger::format_message(LogLevel level, const std::string& message, bool useColors) const {
            std::stringstream ss;
            
            ss << "[" << timestamp() << "] ";
            
            if (useColors) {
                ss << level_color(level);
            }
            
            ss << "[" << level_name(level) << "]";
            
            if (useColors) {
                ss << color_reset();
            }
            
            ss << " [" << m_loggerName << "] ";
            
            ss << message;
            
            ss << "\n";
            
            return ss.str();
        }

        std::string_view Logger::level_name(LogLevel level) const {
            switch (level) {
                case LogLevel::Debug:   return "debug";
                case LogLevel::Info:    return "info ";
                case LogLevel::Warning: return "warn ";
                case LogLevel::Error:   return "error";
                default:                return "  ?  ";
            }
        }

        std::string_view Logger::level_color(LogLevel level) const {
            switch (level) {
                case LogLevel::Debug:   return "\033[36m";
                case LogLevel::Info:    return "\033[32m";
                case LogLevel::Warning: return "\033[33m";
                case LogLevel::Error:   return "\033[31m";
                default:                return "\033[0m";
            }
        }

        void Logger::rotate_log_file(LogTarget& target) {
            if (!target.is_file()) {
                return;
            }

            size_t currentSize = target.file_size();
            if (currentSize < m_maxFileSize) {
                return;
            }

            const std::filesystem::path& basePath = target.file_path();
            
            {
                std::lock_guard lock(target.mutex());
                if (target.file_stream()) {
                    target.file_stream().close();
                }
            }

            for (size_t i = m_maxFiles - 1; i > 0; --i) {
                std::filesystem::path oldPath = basePath;
                oldPath += "." + std::to_string(i);
                
                std::filesystem::path newPath = basePath;
                newPath += "." + std::to_string(i + 1);
                
                if (std::filesystem::exists(oldPath)) {
                    if (i + 1 >= m_maxFiles) {
                        std::filesystem::remove(oldPath);
                    } else {
                        std::filesystem::rename(oldPath, newPath);
                    }
                }
            }

            std::filesystem::path rotatedPath = basePath;
            rotatedPath += ".1";
            if (std::filesystem::exists(basePath)) {
                std::filesystem::rename(basePath, rotatedPath);
            }

            {
                std::lock_guard lock(target.mutex());
                target.file_stream() = std::ofstream(
                    basePath,
                    std::ios::app | std::ios::out
                );
            }
        }

        std::string Logger::timestamp() const {
            auto now = std::chrono::system_clock::now();
            auto time = std::chrono::system_clock::to_time_t(now);
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()
            ) % 1000;

            std::stringstream ss;
            ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
            ss << "." << std::setfill('0') << std::setw(3) << ms.count();
            
            return ss.str();
        }
    }
}

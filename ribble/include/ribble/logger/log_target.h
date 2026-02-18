#ifndef RIBBLE_LOG_TARGET_H
#define RIBBLE_LOG_TARGET_H

#include <filesystem>
#include <fstream>
#include <iosfwd>
#include <memory>
#include <mutex>
#include <string>

namespace ribble::logger {
    class LogTarget {
    public:
        static LogTarget File(const std::filesystem::path& filePath);

        static LogTarget Stream(std::ostream& stream);

        LogTarget(LogTarget&&) noexcept = default;
        LogTarget& operator=(LogTarget&&) noexcept = default;
        LogTarget(const LogTarget&) = delete;
        LogTarget& operator=(const LogTarget&) = delete;

        void write(const std::string& formattedMessage) const;

        void flush() const;

        [[nodiscard]] bool is_file() const { return m_isFile; }

        [[nodiscard]] const std::filesystem::path& file_path() const { return m_filePath; }

        [[nodiscard]] size_t file_size() const;

        [[nodiscard]] std::mutex& mutex() const;
        [[nodiscard]] std::ofstream& file_stream() const;

    private:
        LogTarget() = default;

        bool m_isFile = false;
        std::filesystem::path m_filePath;
        std::unique_ptr<std::ofstream> m_fileStream;
        std::ostream* m_ostream = nullptr;
        mutable std::shared_ptr<std::mutex> m_mutex = std::make_shared<std::mutex>();
    };
}

#endif // RIBBLE_LOG_TARGET_H


#include <ribble/logger/log_target.h>

#include <filesystem>
#include <fstream>
#include <stdexcept>

namespace ribble::logger {
    LogTarget LogTarget::File(const std::filesystem::path& filePath) {
        LogTarget target;
        target.m_isFile = true;
        target.m_filePath = filePath;
        
        if (auto parent = filePath.parent_path(); !parent.empty()) {
            std::filesystem::create_directories(parent);
        }
        
        target.m_fileStream = std::make_unique<std::ofstream>(
            filePath,
            std::ios::app | std::ios::out
        );
        
        if (!target.m_fileStream->is_open()) {
            throw std::runtime_error("Failed to open log file: " + filePath.string());
        }
        
        return target;
    }

    LogTarget LogTarget::Stream(std::ostream& stream) {
        LogTarget target;
        target.m_isFile = false;
        target.m_ostream = &stream;
        return target;
    }

    void LogTarget::write(const std::string& formattedMessage) const {
        std::lock_guard<std::mutex> lock(*m_mutex);
        if (m_isFile && m_fileStream && m_fileStream->is_open()) {
            *m_fileStream << formattedMessage;
        } else if (!m_isFile && m_ostream) {
            *m_ostream << formattedMessage;
        }
    }

    void LogTarget::flush() const {
        std::lock_guard<std::mutex> lock(*m_mutex);
        if (m_isFile && m_fileStream && m_fileStream->is_open()) {
            m_fileStream->flush();
        } else if (!m_isFile && m_ostream) {
            m_ostream->flush();
        }
    }

    size_t LogTarget::file_size() const {
        std::lock_guard<std::mutex> lock(*m_mutex);
        if (m_isFile && std::filesystem::exists(m_filePath)) {
            return std::filesystem::file_size(m_filePath);
        }
        return 0;
    }

    std::mutex& LogTarget::mutex() const {
        return *m_mutex;
    }

    std::ofstream& LogTarget::file_stream() const {
        return *m_fileStream;
    }
}


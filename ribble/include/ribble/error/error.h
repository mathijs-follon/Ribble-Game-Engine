//
// Created by Mathijs Follon on 2/18/26.
//

#ifndef RIBBLE_ERROR_H
#define RIBBLE_ERROR_H
#include <string>

namespace ribble::error {

    enum class Failure {
        Logger_FileNotFound,
    };

    class Error {
    public:
        explicit Error(std::string&& message, Failure failure, bool isFatal = false, const char *fileName = nullptr, size_t fileLine = 0);

        [[nodiscard]] const char* file_name() const;
        [[nodiscard]] size_t file_line() const;
        [[nodiscard]] std::string_view message() const;
        [[nodiscard]] bool is_fatal() const;
        [[nodiscard]] Failure failure() const;

    private:
        const char* m_fileName;
        size_t m_fileLine;
        std::string m_message;
        bool m_isFatal;
        Failure m_failure;
    };
}

#endif //RIBBLE_ERROR_H
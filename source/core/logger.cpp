#include "ribble/core/logger.h"
#include <algorithm>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <spdlog/details/log_msg.h>
#include <spdlog/sinks/ansicolor_sink.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <sstream>
#include <utility>
#include <vector>

namespace ribble {
    namespace {
        std::shared_ptr<spdlog::logger> g_logger = nullptr;
        std::string g_logDirectory;
        std::mutex g_loggerMutex;
        std::once_flag g_initFlag;
        std::atomic<bool> g_initialized{false};

        template<typename Sink>
        class level_filter_sink : public spdlog::sinks::base_sink<std::mutex> {
        public:
            explicit level_filter_sink(std::shared_ptr<Sink> sink,
                                       std::vector<spdlog::level::level_enum> allowed_levels) :
                sink_(std::move(sink)), allowed_levels_(std::move(allowed_levels)) {}

        protected:
            void sink_it_(const spdlog::details::log_msg &msg) override {
                if (std::ranges::find(allowed_levels_, msg.level) != allowed_levels_.end()) {
                    sink_->log(msg);
                }
            }

            void flush_() override { sink_->flush(); }

        private:
            std::shared_ptr<Sink> sink_;
            std::vector<spdlog::level::level_enum> allowed_levels_;
        };

    } // namespace

    bool InitializeLogger(const std::string &logDirectory, size_t maxFileSize, size_t maxFiles) {

        if (g_initialized.load(std::memory_order_acquire)) {
            return true;
        }

        bool initSuccess = false;
        std::call_once(g_initFlag, [&]() {
            std::lock_guard<std::mutex> lock(g_loggerMutex);

            if (g_logger != nullptr) {
                initSuccess = true;
                return;
            }

            try {
                g_logDirectory = logDirectory;

                std::filesystem::path logPath(logDirectory);
                if (!std::filesystem::exists(logPath)) {
                    std::filesystem::create_directories(logPath);
                }

                auto now = std::chrono::system_clock::now();
                auto time = std::chrono::system_clock::to_time_t(now);
                std::stringstream ss;
                ss << std::put_time(std::localtime(&time), "%Y-%m-%d_%H-%M-%S");
                std::string timestamp = ss.str();

                std::filesystem::path latestLogPath = logPath / "latest.log";
                std::filesystem::path errorLatestLogPath = logPath / "error.latest.log";
                std::filesystem::path datedLogPath = logPath / (timestamp + ".log");
                std::filesystem::path errorDatedLogPath = logPath / ("error." + timestamp + ".log");

                auto stdoutBaseSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
                stdoutBaseSink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
                auto stdoutSink = std::make_shared<level_filter_sink<spdlog::sinks::stdout_color_sink_mt>>(
                        stdoutBaseSink,
                        std::vector<spdlog::level::level_enum>{spdlog::level::debug, spdlog::level::info});

                auto stderrBaseSink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
                stderrBaseSink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
                auto stderrSink = std::make_shared<level_filter_sink<spdlog::sinks::stderr_color_sink_mt>>(
                        stderrBaseSink,
                        std::vector<spdlog::level::level_enum>{spdlog::level::warn, spdlog::level::err});

                auto rotatingSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(latestLogPath.string(),
                                                                                           maxFileSize, maxFiles);
                rotatingSink->set_level(spdlog::level::trace);
                rotatingSink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");

                auto errorRotatingSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                        errorLatestLogPath.string(), maxFileSize, maxFiles);
                errorRotatingSink->set_level(spdlog::level::err);
                errorRotatingSink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");

                auto datedSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(datedLogPath.string());
                datedSink->set_level(spdlog::level::trace);
                datedSink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");

                auto errorDatedSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(errorDatedLogPath.string());
                errorDatedSink->set_level(spdlog::level::err);
                errorDatedSink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");

                std::vector<spdlog::sink_ptr> sinks = {stdoutSink,        stderrSink, rotatingSink,
                                                       errorRotatingSink, datedSink,  errorDatedSink};

                g_logger = std::make_shared<spdlog::logger>("ribble", sinks.begin(), sinks.end());

#ifdef RIBBLE_DEBUG
                g_logger->set_level(spdlog::level::debug);
#else
                g_logger->set_level(spdlog::level::info);
#endif

                g_logger->flush_on(spdlog::level::warn);

                g_logger->info("Logger initialized - Log directory: {}", logDirectory);
                g_logger->info("Log files: latest.log (rotating), error.latest.log (rotating), {} (dated), {} (dated)",
                               datedLogPath.filename().string(), errorDatedLogPath.filename().string());
                g_logger->flush();
                std::cout.flush();
                std::cerr.flush();
                std::atomic_thread_fence(std::memory_order_seq_cst);

                spdlog::drop("ribble");
                spdlog::register_logger(g_logger);
                spdlog::set_default_logger(g_logger);

                g_initialized.store(true, std::memory_order_release);
                initSuccess = true;
            } catch (const std::exception &e) {
                std::cerr << "Failed to initialize logger: " << e.what() << std::endl;
                initSuccess = false;
            }
        });

        return initSuccess;
    }

    void ShutdownLogger() {
        std::lock_guard<std::mutex> lock(g_loggerMutex);
        if (g_logger) {
            g_logger->info("Shutting down logger");
            g_logger->flush();
            spdlog::drop_all();
            g_logger.reset();
            g_initialized.store(false, std::memory_order_release);
        }
    }

    std::shared_ptr<spdlog::logger> GetLogger() {
        if (g_initialized.load(std::memory_order_acquire)) {
            std::lock_guard<std::mutex> lock(g_loggerMutex);
            return g_logger;
        }

        return nullptr;
    }

    void SetLogLevel(spdlog::level::level_enum level) {
        std::lock_guard<std::mutex> lock(g_loggerMutex);
        if (g_logger) {
            g_logger->set_level(level);
            g_logger->info("Log level set to: {}", spdlog::level::to_string_view(level));
        }
    }

    spdlog::level::level_enum GetLogLevel() {
        std::lock_guard<std::mutex> lock(g_loggerMutex);
        if (g_logger) {
            return g_logger->level();
        }
#ifdef RIBBLE_DEBUG
        return spdlog::level::debug;
#else
        return spdlog::level::info;
#endif
    }

    namespace detail {
        void LogDebugSync(const std::string &message) {
            std::lock_guard<std::mutex> lock(g_loggerMutex);
            if (g_logger) {
                g_logger->debug(message);
                g_logger->flush();
                std::cout.flush();
                std::cerr.flush();
            }
        }

        void LogInfoSync(const std::string &message) {
            std::lock_guard<std::mutex> lock(g_loggerMutex);
            if (g_logger) {
                g_logger->info(message);
            }
        }

        void LogWarningSync(const std::string &message) {
            std::lock_guard<std::mutex> lock(g_loggerMutex);
            if (g_logger) {
                g_logger->warn(message);
                g_logger->flush();
                std::cout.flush();
                std::cerr.flush();
            }
        }

        void LogErrorSync(const std::string &message) {
            std::lock_guard<std::mutex> lock(g_loggerMutex);
            if (g_logger) {
                g_logger->error(message);
                g_logger->flush();
                std::cout.flush();
                std::cerr.flush();
            }
        }
    } // namespace detail
} // namespace ribble

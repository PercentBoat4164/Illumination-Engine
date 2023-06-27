#include "Logger.hpp"

// External dependencies
#include <spdlog/async.h>
#include <spdlog/common.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

// Standard library dependencies
#include <memory>
#include <mutex>
#include <vector>

/* See Logger.hpp for variable documentation. */;
std::shared_ptr<spdlog::sinks::basic_file_sink_st> IE::Core::Logger::m_logFileSink{};
std::unique_ptr<std::mutex>                        IE::Core::Logger::m_logMutex{};

/* See Logger.hpp for method documentation. */;
IE::Core::Logger::Logger(const std::filesystem::path &t_filePath) {
    // Used to determine when the 'Start of new log' message should be displayed, and when the `m_logFileSink`
    // should be initialized.
    static bool init{false};
    m_logMutex = std::make_unique<std::mutex>();
    if (!init)  // Only run the first time that a logger is created.
        m_logFileSink = std::make_shared<spdlog::sinks::basic_file_sink_st>(
          std::filesystem::path("logs") / "IlluminationEngine.log"
        );
    // Compiled list of sinks that this logger will log to. Always includes the `m_logFileSink` and stdout.
    std::vector<spdlog::sink_ptr> sinks{
      m_logFileSink,
      std::make_shared<spdlog::sinks::stdout_color_sink_st>(),
      // Create the file sink that will be used to write to `t_filePath`.
      std::make_shared<spdlog::sinks::basic_file_sink_st>(
        std::filesystem::path("logs") / (t_filePath.string() + ".log")
      )};
    // Create the logger from the list of sinks.
    m_logger = std::make_shared<spdlog::logger>(t_filePath, sinks.begin(), sinks.end());
    setLogLevel(ILLUMINATION_ENGINE_LOG_LEVEL_TRACE);
    if (!init) {
        log("START OF NEW LOG");
        init = true;
    }
    log("Created " + t_filePath.string() + " logger.");
}

void IE::Core::Logger::log(const std::string &t_message, IE::Core::Logger::Level t_level) const {
    /** @todo Try to remove the need for `m_logMutex`. **/
    std::lock_guard<std::mutex> lock(*m_logMutex);
    m_logger->log((spdlog::level::level_enum) t_level, t_message);
}

void IE::Core::Logger::log(const std::string &t_message, VkDebugUtilsMessageSeverityFlagBitsEXT t_level) const {
    std::lock_guard<std::mutex> lock(*m_logMutex);
    // Map Vulkan log levels to Illumination Engine log levels.
    Level                       logLevel{};
    switch (t_level) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            logLevel = ILLUMINATION_ENGINE_LOG_LEVEL_TRACE;
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: logLevel = ILLUMINATION_ENGINE_LOG_LEVEL_INFO; break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: logLevel = ILLUMINATION_ENGINE_LOG_LEVEL_WARN; break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: logLevel = ILLUMINATION_ENGINE_LOG_LEVEL_ERROR; break;
        default: logLevel = ILLUMINATION_ENGINE_LOG_LEVEL_TRACE; break;
    }
    m_logger->log((spdlog::level::level_enum) logLevel, t_message);
}

void IE::Core::Logger::setLogLevel(IE::Core::Logger::Level t_level) {
    m_logger->set_level((spdlog::level::level_enum) t_level);
}

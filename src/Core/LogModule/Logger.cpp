#include "Logger.hpp"

#include <memory>
#include <mutex>
#include <spdlog/async.h>
#include <spdlog/common.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <vector>

bool                                               IE::Core::Logger::m_init{false};
std::shared_ptr<spdlog::sinks::basic_file_sink_st> IE::Core::Logger::m_logFileSink{};
std::unique_ptr<std::mutex>                        IE::Core::Logger::m_logMutex{};

IE::Core::Logger::Logger(const std::filesystem::path &t_filePath) {
    m_logMutex = std::make_unique<std::mutex>();
    if (!m_init)
        m_logFileSink = std::make_shared<spdlog::sinks::basic_file_sink_st>(
          std::filesystem::path("logs") / "IlluminationEngine.log"
        );
    std::vector<spdlog::sink_ptr> sinks{
      m_logFileSink,
      std::make_shared<spdlog::sinks::stdout_color_sink_st>(),
      std::make_shared<spdlog::sinks::basic_file_sink_st>(
        std::filesystem::path("logs") / (t_filePath.string() + ".log")
      )};
    m_logger = std::make_shared<spdlog::logger>(t_filePath, sinks.begin(), sinks.end());
    setLogLevel(ILLUMINATION_ENGINE_LOG_LEVEL_TRACE);
    if (!m_init) log("START OF NEW LOG");
    m_init = true;
    log("Created logger");
}

void IE::Core::Logger::log(const std::string &t_message, IE::Core::Logger::Level t_level) const {
    std::lock_guard<std::mutex> lock(*m_logMutex);
    m_logger->log((spdlog::level::level_enum) t_level, t_message);
}

void IE::Core::Logger::log(const std::string &t_message, VkDebugUtilsMessageSeverityFlagBitsEXT t_level) const {
    std::lock_guard<std::mutex> lock(*m_logMutex);
    switch (t_level) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            m_logger->log(spdlog::level::trace, t_message);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: m_logger->log(spdlog::level::info, t_message); break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: m_logger->log(spdlog::level::warn, t_message); break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: m_logger->log(spdlog::level::err, t_message); break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT:
        default: m_logger->log(spdlog::level::trace, t_message); break;
    }
}

void IE::Core::Logger::setLogLevel(IE::Core::Logger::Level t_level) {
    m_logger->set_level((spdlog::level::level_enum) t_level);
}

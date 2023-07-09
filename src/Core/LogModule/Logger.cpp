#include "Logger.hpp"

#include "Core/Core.hpp"

#include <memory>
#include <mutex>
#include <spdlog/async.h>
#include <spdlog/common.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <vector>

bool                                               IE::Core::Logger::m_init{false};
std::shared_ptr<spdlog::sinks::basic_file_sink_st> IE::Core::Logger::m_logFileSink{};
// This does not get constructed before the Logger() constructor is called on some Windows compilers. This making
// it a pointer then having it point to a mutex that is created in the constructor solves that problem. There is no
// need to delete this pointer as the pointer and the object it is pointing to should last for the entire duration
// of the program.
std::mutex                                        *IE::Core::Logger::m_logMutex{};

IE::Core::Logger::Logger(const std::string &t_name) {
    m_logMutex = new std::mutex();
    if (!m_init)
        m_logFileSink = std::make_shared<spdlog::sinks::basic_file_sink_st>(
          IE::Core::Core::getFileSystem()->getInternalLogDirectory() / "IlluminationEngine.log"
        );
    std::vector<spdlog::sink_ptr> sinks{m_logFileSink, std::make_shared<spdlog::sinks::stdout_color_sink_st>()};
    m_logger = std::make_shared<spdlog::logger>(t_name, sinks.begin(), sinks.end());
    setLogLevel(ILLUMINATION_ENGINE_LOG_LEVEL_TRACE);
    if (!m_init)
        log(
          "\n\n\n================================================================================================="
          "===\n========================================= START OF NEW LOG "
          "=========================================\n============================================================"
          "========================================\n\n"
        );
    m_init = true;
    log("Created logger");
}

void IE::Core::Logger::log(const std::string &t_msg, IE::Core::Logger::Level t_level) const {
    std::lock_guard<std::mutex> lock(*m_logMutex);
    m_logger->log((spdlog::level::level_enum) t_level, t_msg);
}

void IE::Core::Logger::log(VkDebugUtilsMessageSeverityFlagBitsEXT t_level, const std::string &t_msg) const {
    std::lock_guard<std::mutex> lock(*m_logMutex);
    switch (t_level) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: m_logger->log(spdlog::level::trace, t_msg); break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: m_logger->log(spdlog::level::info, t_msg); break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: m_logger->log(spdlog::level::warn, t_msg); break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: m_logger->log(spdlog::level::err, t_msg); break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT:
        default: m_logger->log(spdlog::level::trace, t_msg); break;
    }
}

void IE::Core::Logger::setLogLevel(IE::Core::Logger::Level t_level) {
    m_logger->set_level((spdlog::level::level_enum) t_level);
}

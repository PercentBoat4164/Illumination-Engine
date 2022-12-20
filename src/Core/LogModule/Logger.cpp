#include "Logger.hpp"

#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

IE::Core::Logger::Logger(const std::string &t_name, const std::string &t_path, Flags t_flags) {
    init();
    std::vector<spdlog::sink_ptr> sinks;
    std::string                   filePath = t_path;
    if (t_flags & IE::Core::Logger::ILLUMINATION_ENGINE_LOG_TO_STDOUT)
        sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
    if (!t_path.empty() || t_flags & IE::Core::Logger::ILLUMINATION_ENGINE_LOG_TO_FILE) {
        if (t_path.empty()) filePath = t_name;
        sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(
          filePath,
          t_flags & ~IE::Core::Logger::ILLUMINATION_ENGINE_LOG_PRESERVE
        ));
    }
    m_logger = std::make_shared<spdlog::async_logger>(t_name, sinks.begin(), sinks.end(), spdlog::thread_pool());
    setLogLevel(ILLUMINATION_ENGINE_LOG_LEVEL_TRACE);
    log("Created logger");
}

void IE::Core::Logger::log(const std::string &t_msg, IE::Core::Logger::Level t_level) const {
    m_logger->log((spdlog::level::level_enum) t_level, t_msg);
}

void IE::Core::Logger::log(VkDebugUtilsMessageSeverityFlagBitsEXT t_level, const std::string &t_msg) const {
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

void IE::Core::Logger::init() {
    if (!m_init) spdlog::init_thread_pool(8192, 2);
    m_init = true;
}

bool IE::Core::Logger::m_init{false};
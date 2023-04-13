#pragma once

#include <filesystem>
#include <memory>
#include <spdlog/logger.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <string>
#include <vulkan/vulkan_core.h>

namespace IE::Core {
/**
 * @brief A small logging tool that is a very thin wrapper for spdlog.
 * @class Logger
 */
class Logger {
private:
    std::shared_ptr<spdlog::logger>                           m_logger;
    static bool                                               m_init;
    static std::shared_ptr<spdlog::sinks::basic_file_sink_st> m_logFileSink;
    static std::mutex                                        *m_logMutex;

public:
    enum Level {
        ILLUMINATION_ENGINE_LOG_LEVEL_OFF             = spdlog::level::off,
        ILLUMINATION_ENGINE_LOG_LEVEL_TRACE           = spdlog::level::trace,
        ILLUMINATION_ENGINE_LOG_LEVEL_DEBUG           = spdlog::level::debug,
        ILLUMINATION_ENGINE_LOG_LEVEL_INFO            = spdlog::level::info,
        ILLUMINATION_ENGINE_LOG_LEVEL_WARN            = spdlog::level::warn,
        ILLUMINATION_ENGINE_LOG_LEVEL_ERROR           = spdlog::level::err,
        ILLUMINATION_ENGINE_LOG_LEVEL_CRITICAL        = spdlog::level::critical,
        ILLUMINATION_ENGINE_LOG_LEVEL_MAX             = spdlog::level::n_levels,
        ILLUMINATION_ENGINE_LOG_LEVEL_0               = spdlog::level::off,
        ILLUMINATION_ENGINE_LOG_LEVEL_1               = spdlog::level::trace,
        ILLUMINATION_ENGINE_LOG_LEVEL_2               = spdlog::level::debug,
        ILLUMINATION_ENGINE_LOG_LEVEL_3               = spdlog::level::info,
        ILLUMINATION_ENGINE_LOG_LEVEL_4               = spdlog::level::warn,
        ILLUMINATION_ENGINE_LOG_LEVEL_5               = spdlog::level::err,
        ILLUMINATION_ENGINE_LOG_LEVEL_6               = spdlog::level::critical,
        ILLUMINATION_ENGINE_LOG_LEVEL_SEVERITY_LOW    = spdlog::level::warn,
        ILLUMINATION_ENGINE_LOG_LEVEL_SEVERITY_MEDIUM = spdlog::level::err,
        ILLUMINATION_ENGINE_LOG_LEVEL_SEVERITY_HIGH   = spdlog::level::critical,
    };

    explicit Logger(const std::string &t_name);
    void
    log(const std::string &t_msg, IE::Core::Logger::Level t_level = ILLUMINATION_ENGINE_LOG_LEVEL_TRACE) const;

    void log(VkDebugUtilsMessageSeverityFlagBitsEXT t_level, const std::string &t_msg) const;

    void setLogLevel(Level t_level);
};
}  // namespace IE::Core
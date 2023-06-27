#pragma once

/**
 * This file contains the Logger class for logging messages.
 */

// External dependencies
#include <spdlog/logger.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <vulkan/vulkan_core.h>

// Standard library dependencies
#include <filesystem>
#include <memory>
#include <string>

/** @namespace IE::Core */
namespace IE::Core {
/**
 * @class Logger
 * @short A small logging tool that is a very thin wrapper for spdlog.
 *
 * Each logger will log to stdout and "logs/IlluminationEngine.log" in addition to the location specified by the
 * constructor.
 */
class Logger {
private:
    // Used to allow copying loggers between pointers.
    std::shared_ptr<spdlog::logger>                           m_logger;
    // Used to give all loggers the 'logs/IlluminationEngine.log' file sink.
    static std::shared_ptr<spdlog::sinks::basic_file_sink_st> m_logFileSink;
    // Used to ensure synchronous access to the m_logger. Needs to be a pointer type because an instance of the
    // Logger is statically constructed before Logger.cpp is compiled. This means that in order to initialize the
    // m_logMutex, we must initialize in the Logger's constructor. In order to do that it must be copy
    // constructable.
    static std::unique_ptr<std::mutex>                        m_logMutex;

public:
    /**
     * @enum Level
     * @short Contains enumerations of the log level.
     *
     * <p> The log levels are broken up into three sections: adjectival, numerical, and low-high. The sections are
     * OFF to MAX, 0 to 6, LOW to HIGH.
     */
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

    /**
     * @short Each logger will log to stdout and "logs/IlluminationEngine.log" in addition to
     * "logs/{t_filePath}.log".
     * @param t_filePath The name of the file that will be logged to in addition to IlluminationEngine and stdout.
     */
    explicit Logger(const std::filesystem::path &t_filePath);

    /**
     * @short Logs t_message to this logger's log file, "logs/IlluminationEngine.log", and stdout.
     * @param t_message The message to be logged.
     * @param t_level The log level this message should be recorded at.
     * @returns void
     */
    void
    log(const std::string &t_message, IE::Core::Logger::Level t_level = ILLUMINATION_ENGINE_LOG_LEVEL_TRACE) const;

    /**
     * @short Logs a message with Vulkan message severity flags.
     *
     * <p> Intended for use only with the Vulkan Validation Layer system.
     * <p> Logs a message in the same way as const the other log function, but is designed to take Vulkan message
     * severity flags.
     * @todo Add `@ link` context to 'the other log function' once CLion's rendered documentation no longer breaks
     * it.
     * @param t_message The message to be logged.
     * @param t_level A Vulkan expression of the log level this message should be recorded at.
     * @returns void
     * @see log(const std::string &, IE::Core::Logger::Level) const
     */
    void log(const std::string &t_message, VkDebugUtilsMessageSeverityFlagBitsEXT t_level) const;

    /**
     * @short Sets the lower bound of log level that this logger will record.
     *
     * <p> This function sets a filter. Anything that has a log level lesser than <code>t_level</code> will be
     * ignored by the logger. This can be used to filter out debug, info or trace messages if they are not needed.
     * @param t_level The lowest log level that should be accepted.
     * @returns void
     */
    void setLogLevel(Level t_level);
};
}  // namespace IE::Core
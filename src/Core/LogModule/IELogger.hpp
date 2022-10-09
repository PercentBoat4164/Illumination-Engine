#pragma once

#include <include/spdlog/logger.h>
#include <memory>
#include <string>

// Illumination Engine replacements for spdlog log levels.
#define ILLUMINATION_ENGINE_LOG_LEVEL_TRACE           spdlog::level::trace
#define ILLUMINATION_ENGINE_LOG_LEVEL_DEBUG           spdlog::level::debug
#define ILLUMINATION_ENGINE_LOG_LEVEL_INFO            spdlog::level::info
#define ILLUMINATION_ENGINE_LOG_LEVEL_WARN            spdlog::level::warn
#define ILLUMINATION_ENGINE_LOG_LEVEL_ERROR           spdlog::level::err
#define ILLUMINATION_ENGINE_LOG_LEVEL_CRITICAL        spdlog::level::critical
#define ILLUMINATION_ENGINE_LOG_LEVEL_OFF             spdlog::level::off
#define ILLUMINATION_ENGINE_LOG_LEVEL_MAX             spdlog::level::n_levels
#define ILLUMINATION_ENGINE_LOG_LEVEL_0               ILLUMINATION_ENGINE_LOG_LEVEL_TRACE
#define ILLUMINATION_ENGINE_LOG_LEVEL_1               ILLUMINATION_ENGINE_LOG_LEVEL_DEBUG
#define ILLUMINATION_ENGINE_LOG_LEVEL_2               ILLUMINATION_ENGINE_LOG_LEVEL_INFO
#define ILLUMINATION_ENGINE_LOG_LEVEL_3               ILLUMINATION_ENGINE_LOG_LEVEL_WARN
#define ILLUMINATION_ENGINE_LOG_LEVEL_4               ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
#define ILLUMINATION_ENGINE_LOG_LEVEL_5               ILLUMINATION_ENGINE_LOG_LEVEL_CRITICAL
#define ILLUMINATION_ENGINE_LOG_LEVEL_6               ILLUMINATION_ENGINE_LOG_LEVEL_OFF
#define ILLUMINATION_ENGINE_LOG_LEVEL_SEVERITY_LOW    ILLUMINATION_ENGINE_LOG_LEVEL_3
#define ILLUMINATION_ENGINE_LOG_LEVEL_SEVERITY_MEDIUM ILLUMINATION_ENGINE_LOG_LEVEL_4
#define ILLUMINATION_ENGINE_LOG_LEVEL_SEVERITY_HIGH   ILLUMINATION_ENGINE_LOG_LEVEL_5

#define ILLUMINATION_ENGINE_DEFAULT_LOGGER_NAME  "Illumination Engine Default Logger"
#define ILLUMINATION_ENGINE_DEFAULT_LOG_FILENAME "IlluminationEngine.log"

/**
 * @brief A small logging tool that is a very thin wrapper for spdlog.
 * @class IELogger
 */
class IELogger {
public:
    IELogger(const std::string &name, const std::string &path);

    explicit IELogger();

    void log(spdlog::level::level_enum level, const std::string &msg) const;

private:
    std::shared_ptr<spdlog::logger> logger;
};
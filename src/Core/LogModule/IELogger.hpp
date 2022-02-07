#pragma once

#include "spdlog/spdlog.h"

//Illumination Engine replacements for spdlog log levels.
#define ILLUMINATION_ENGINE_LOG_LEVEL_TRACE spdlog::level::trace
#define ILLUMINATION_ENGINE_LOG_LEVEL_DEBUG spdlog::level::debug
#define ILLUMINATION_ENGINE_LOG_LEVEL_INFO spdlog::level::info
#define ILLUMINATION_ENGINE_LOG_LEVEL_WARN spdlog::level::warn
#define ILLUMINATION_ENGINE_LOG_LEVEL_ERROR spdlog::level::err
#define ILLUMINATION_ENGINE_LOG_LEVEL_CRITICAL spdlog::level::critical
#define ILLUMINATION_ENGINE_LOG_LEVEL_OFF spdlog::level::off
#define ILLUMINATION_ENGINE_LOG_LEVEL_MAX spdlog::level::n_levels
#define ILLUMINATION_ENGINE_LOG_LEVEL_0 ILLUMINATION_ENGINE_LOG_LEVEL_TRACE
#define ILLUMINATION_ENGINE_LOG_LEVEL_1 ILLUMINATION_ENGINE_LOG_LEVEL_DEBUG
#define ILLUMINATION_ENGINE_LOG_LEVEL_2 ILLUMINATION_ENGINE_LOG_LEVEL_INFO
#define ILLUMINATION_ENGINE_LOG_LEVEL_3 ILLUMINATION_ENGINE_LOG_LEVEL_WARN
#define ILLUMINATION_ENGINE_LOG_LEVEL_4 ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
#define ILLUMINATION_ENGINE_LOG_LEVEL_5 ILLUMINATION_ENGINE_LOG_LEVEL_CRITICAL
#define ILLUMINATION_ENGINE_LOG_LEVEL_6 ILLUMINATION_ENGINE_LOG_LEVEL_OFF
#define ILLUMINATION_ENGINE_LOG_LEVEL_SEVERITY_LOW ILLUMINATION_ENGINE_LOG_LEVEL_3
#define ILLUMINATION_ENGINE_LOG_LEVEL_SEVERITY_MEDIUM ILLUMINATION_ENGINE_LOG_LEVEL_4
#define ILLUMINATION_ENGINE_LOG_LEVEL_SEVERITY_HIGH ILLUMINATION_ENGINE_LOG_LEVEL_5

#define ILLUMINATION_ENGINE_DEFAULT_LOGGER_NAME "Illumination Engine Default Logger"


/**
 * @brief A small logging tool that is a very thin wrapper for spdlog.
 * @class IELogger
 */
class IELogger : public spdlog::logger{
public:
    IELogger() : spdlog::logger(ILLUMINATION_ENGINE_DEFAULT_LOGGER_NAME) {}

    explicit IELogger(spdlog::logger* initialMasterLogger) : spdlog::logger(ILLUMINATION_ENGINE_DEFAULT_LOGGER_NAME) {
        masterLogger = initialMasterLogger;
    }

    explicit IELogger(const std::string &name) : spdlog::logger(name) {}

    explicit IELogger(const std::string &name, spdlog::logger* initialMasterLogger) : spdlog::logger(name) {
        masterLogger = initialMasterLogger;
    }

    IELogger(const std::string &name, const spdlog::sinks_init_list &sinks) : spdlog::logger(name, sinks) {}

    IELogger(const std::string &name, const spdlog::sinks_init_list &sinks, spdlog::logger* initialMasterLogger) : spdlog::logger(name, sinks) {
        masterLogger = initialMasterLogger;
    }

    explicit IELogger(const logger &other) : spdlog::logger(other) {}

    explicit IELogger(logger &&other) : spdlog::logger(other) {}

    #pragma clang diagnostic push
    #pragma ide diagnostic ignored "HidingNonVirtualFunction"
    template<typename T> void log(spdlog::level::level_enum level, const T& msg) {
        spdlog::logger::log(level, msg);
        if (masterLogger) {
            masterLogger->log(level, msg);
        }
    }
    #pragma clang diagnostic pop

    template<typename T> void log(spdlog::source_loc loc, const T& msg) {
        spdlog::logger::log(loc, msg);
        if (masterLogger) {
            masterLogger->log(loc, msg);
        }
    }

    template<typename T> void log(spdlog::log_clock::time_point timePoint, const T& msg) {
        spdlog::logger::log(timePoint, msg);
        if (masterLogger) {
            masterLogger->log(timePoint, msg);
        }
    }

    template<typename T> static void logToMasterLogger(spdlog::level::level_enum level, const T& msg) {
        if (masterLogger) {
            masterLogger->log(level, msg);
        }
    }

private:
    static spdlog::logger* masterLogger;
};
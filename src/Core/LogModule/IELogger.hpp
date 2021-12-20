#pragma once

#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"

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


/**
 * @brief A small logging tool that is a very thin wrapper for spdlog.
 * @class IELogger
 */
class IELogger {
public:
    /**
     * @brief Create a loggers named "IE default logger" that logs to "logs/IEDefaultLog.log"
     */
    IELogger() {
        loggers.push_back(spdlog::basic_logger_mt("IE default logger", "logs/IEDefaultLog.log", true));
        spdlog::set_default_logger(loggers[0]);
    }

    /**
     * @brief Create a loggers that is named <name> and logs to "logs/<name>.log".
     * @param name
     */
    explicit IELogger(const std::string& name) {
        loggers.push_back(spdlog::basic_logger_mt(name, "logs/" + name + ".log", true));
        spdlog::set_default_logger(loggers[0]);
    };

    /**
     * @brief Create a loggers that is named <name> and logs to "logs/<file>.log".
     * @param name
     * @param file
     */
    IELogger(const std::string& name, const std::string& file) {
        loggers.push_back(spdlog::basic_logger_mt(name, "logs/" + file + ".log", true));
        spdlog::set_default_logger(loggers[0]);
    };

    void logAll(spdlog::level::level_enum level, const std::string& message, const std::string& name="") {
        for (const std::shared_ptr<spdlog::logger>& logger : loggers) {
            if (name.empty() || logger->name() == name) {
                logger->log(level, message);
            }
        }
    }

    void addLog(const std::string& name) {
        loggers.push_back(spdlog::basic_logger_mt(name, "logs/" + name + ".log", true));
    }

    void addLog(const std::string& name, const std::string& file) {
        loggers.push_back(spdlog::basic_logger_mt(name, "logs/" + file + ".log", true));
    }

    static void logDefault(spdlog::level::level_enum level, const std::string& message) {
        spdlog::log(level, message);
    }

    void operator()(spdlog::level::level_enum level, const std::string& message) {
        logAll(level, message);
    }

    std::shared_ptr<spdlog::logger> operator[](uint32_t index) {
        return loggers[index];
    }

private:
    std::vector<std::shared_ptr<spdlog::logger>> loggers;
};
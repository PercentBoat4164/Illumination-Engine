#include "IELogger.hpp"

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

IELogger::IELogger(const std::string &name, const std::string &path) {
    logger = spdlog::basic_logger_mt(name, path, true);
}

IELogger::IELogger() {
    logger = spdlog::default_logger();
}

void IELogger::log(spdlog::level::level_enum level, const std::string &msg) const {
    logger->log(level, msg);
}

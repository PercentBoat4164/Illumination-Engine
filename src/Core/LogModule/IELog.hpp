#pragma once

#include "log4cplus/logger.h"
#include <string>
#include <algorithm>

class IELog {
private:
    std::string logName;
    bool created;

public:
    log4cplus::Logger logger;
    std::unordered_map<std::string, log4cplus::SharedAppenderPtr> modules;

    void create(const std::string &name);

    void addModule(const std::string &name);

    void log(const std::string &message, log4cplus::LogLevel logLevel, const std::string &name);

    void cleanLogFiles();
};

#ifdef ILLUMINATION_ENGINE_LOG
#include "log4cplus/loglevel.h"
#include "log4cplus/layout.h"
#include "log4cplus/fileappender.h"
#include "log4cplus/consoleappender.h"

void IELog::create(const std::string &name) {
    logName = name;
    std::replace(logName.begin(), logName.end(), ' ', '_');
    log4cplus::SharedAppenderPtr consoleAppender{new log4cplus::ConsoleAppender()};
    consoleAppender->setName(logName + "Console");
    log4cplus::SharedAppenderPtr fileAppender{new log4cplus::FileAppender(logName + ".log")};
    fileAppender->setName(logName + "File");
    std::unique_ptr<log4cplus::Layout> layout = std::unique_ptr<log4cplus::Layout>(new log4cplus::SimpleLayout);
    consoleAppender->setLayout(reinterpret_cast<std::unique_ptr<log4cplus::Layout> &&>(layout));
    layout = std::unique_ptr<log4cplus::Layout>(new log4cplus::TTCCLayout);
    fileAppender->setLayout(reinterpret_cast<std::unique_ptr<log4cplus::Layout> &&>(layout));
    logger = log4cplus::Logger::getInstance(logName + "Logger");
    logger.addAppender(consoleAppender);
    logger.addAppender(fileAppender);
    created = true;
}

void IELog::addModule(const std::string &name) {
    if (created & (modules.find(name) == modules.end())) {
        std::string nameNoSpace = name;
        std::replace(nameNoSpace.begin(), nameNoSpace.end(), ' ', '_');
        log4cplus::SharedAppenderPtr fileAppender{new log4cplus::FileAppender(logName + "_" + nameNoSpace + ".log")};
        fileAppender->setName(name);
        std::unique_ptr<log4cplus::Layout> layout = std::unique_ptr<log4cplus::Layout>(new log4cplus::TTCCLayout);
        fileAppender->setLayout(reinterpret_cast<std::unique_ptr<log4cplus::Layout> &&>(layout));
        modules[name] = fileAppender;
    }
}

void IELog::log(const std::string &message, log4cplus::LogLevel logLevel, const std::string &name) {
    if (created & (!message.empty())) {
        log4cplus::SharedAppenderPtr module = modules[name];
        if (module == nullptr) { logger.log(log4cplus::WARN_LOG_LEVEL, "UNKNOWN MODULE: " + name + ". The next message will only appear in " + logName + ".log"); } else { logger.addAppender(module); }
        logger.log(logLevel, logName + " | " + name + "\t\t>>> " + message);
        if (module != nullptr) { logger.removeAppender(module); }
    }
}

void IELog::cleanLogFiles() {
    if (created) {
        std::remove((logName + ".log").c_str());
        for (std::pair<std::string, log4cplus::SharedAppenderPtr> module : modules) { std::remove((logName + "_" + module.first + ".log").c_str()); }
    }
}
#else
void Log::create(const std::string &name) {}

void Log::addModule(const std::string &name) {}

void Log::log(const std::string &message, log4cplus::LogLevel logLevel, const std::string &name) {}

void Log::cleanLogFiles() {}
#endif
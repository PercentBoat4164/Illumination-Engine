#pragma once

#include <log4cplus/logger.h>
#include <string>
#include <algorithm>

class Log {
private:
    std::string logName;
public:
    log4cplus::Logger logger;
    std::unordered_map<std::string, log4cplus::SharedAppenderPtr> modules;

    void create(const std::string &name);

    void addModule(const std::string &name);

    void log(const std::string &message, log4cplus::LogLevel logLevel, const std::string &name);
};

#ifdef ILLUMINATION_ENGINE_LOG
#include <log4cplus/loglevel.h>
#include <log4cplus/layout.h>
#include <log4cplus/fileappender.h>
#include <log4cplus/consoleappender.h>

void Log::create(const std::string &name) {
    logName = name;
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
}

void Log::addModule(const std::string &name) {
    if (this != nullptr) {
        if (!modules.contains(name)) {
            std::string logNameNoSpace = logName;
            std::replace(logNameNoSpace.begin(), logNameNoSpace.end(), ' ', '_');
            std::string nameNoSpace = name;
            std::replace(nameNoSpace.begin(), nameNoSpace.end(), ' ', '_');
            log4cplus::SharedAppenderPtr fileAppender{new log4cplus::FileAppender(logNameNoSpace + "_" + nameNoSpace + ".log")};
            fileAppender->setName(name);
            std::unique_ptr<log4cplus::Layout> layout = std::unique_ptr<log4cplus::Layout>(new log4cplus::TTCCLayout);
            fileAppender->setLayout(reinterpret_cast<std::unique_ptr<log4cplus::Layout> &&>(layout));
            modules[name] = fileAppender;
        }
    }
}

void Log::log(const std::string &message, log4cplus::LogLevel logLevel, const std::string &name) {
    if ((this != nullptr) & (!message.empty())) {
        log4cplus::SharedAppenderPtr module = modules[name];
        if (module == nullptr) { logger.log(log4cplus::WARN_LOG_LEVEL, "UNKNOWN MODULE: " + name + ". The next message will only appear in " + logName + ".log"); } else { logger.addAppender(module); }
        logger.log(logLevel, logName + " | " + name + "\t\t>>> " + message);
        if (module != nullptr) { logger.removeAppender(module); }
    }
}
#else
void Log::create(const std::string &name) {}
void Log::addModule(const std::string &name) {}
void Log::log(const std::string &message, log4cplus::LogLevel logLevel, const std::string &name) {}
#endif
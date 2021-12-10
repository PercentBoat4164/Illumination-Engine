#include "IELog.hpp"


/*
 * Any code for testing the log module goes here.
 */
int main(int argc, char **argv) {
    IELog log;
    log.create("TestLog");
    log.addModule("LogModule");
    log.log("TESTING>>>> It works if you can read this.", log4cplus::INFO_LOG_LEVEL, "LogModule");
    log.cleanLogFiles();
}
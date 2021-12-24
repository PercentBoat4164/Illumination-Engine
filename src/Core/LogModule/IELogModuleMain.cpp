#include "IELogger.hpp"

/*
 * Any code for testing the log module goes here.
 */
int main(int argc, char **argv) {
    IELogger::logDefault(ILLUMINATION_ENGINE_LOG_LEVEL_INFO, "This line tests the static function call.");
    IELogger logger{"IEGM"};
    IELogger::logDefault(ILLUMINATION_ENGINE_LOG_LEVEL_DEBUG, "This line tests higher level stuff.");
    logger.logToAll(ILLUMINATION_ENGINE_LOG_LEVEL_INFO, "This line tests the static function call from an object.");
    logger(ILLUMINATION_ENGINE_LOG_LEVEL_CRITICAL, "This line tests the operator overload.");
}
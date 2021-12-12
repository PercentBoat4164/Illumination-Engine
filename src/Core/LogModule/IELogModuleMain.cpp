#include "IELogger.hpp"

/*
 * Any code for testing the log module goes here.
 */
int main(int argc, char **argv) {
    IELogger logger{"IEGM"};
    IELogger::logDefault(ILLUMINATION_ENGINE_LOG_LEVEL_TRACE, "This line tests the static function call.");
    logger.logAll(ILLUMINATION_ENGINE_LOG_LEVEL_0, "This line tests the static function call from an object.");
    logger(spdlog::level::trace, "This line tests the operator overload.");
}
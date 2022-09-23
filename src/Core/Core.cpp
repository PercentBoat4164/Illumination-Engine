#include "Core.hpp"

IE::Core::Core::Core() :
        logger(
          ILLUMINATION_ENGINE_CORE_LOGGER_NAME,
          ILLUMINATION_ENGINE_CORE_LOG_FILENAME,
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_TO_FILE | IE::Core::Logger::ILLUMINATION_ENGINE_LOG_TO_STDOUT
        ),
        enginesMutex(),
        engines() {
    logger.setLogLevel(IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_1);
}

#pragma once

#include "Core/FileSystemModule/IEFileSystem.hpp"
#include "Core/LogModule/IELogger.hpp"
#include "Core/ThreadingModule/ThreadPool/ThreadPool.hpp"
#include "Engine.hpp"

#define ILLUMINATION_ENGINE_CORE_LOGGER_NAME  "Illumination Engine"
#define ILLUMINATION_ENGINE_CORE_LOG_FILENAME "logs/IlluminationEngine.log"

namespace IE::Core {
struct Core {
public:
    IE::Core::Logger    logger;
    std::mutex          enginesMutex;
    std::vector<Engine> engines;
    ThreadPool          threadPool;
    IEFileSystem        filesystem{"res"};
    Core();
};
}  // namespace IE::Core
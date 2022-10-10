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
    IE::Core::Logger                                  logger;
    std::mutex                                        enginesMutex;
    std::unordered_map<std::string, IE::Core::Engine> engines;
    ThreadPool                                        threadPool;
    IEFileSystem                                      filesystem{"res"};

    static IE::Core::Core &getInst() {
        static IE::Core::Core inst{};
        return inst;
    }

private:
    Core();
};
}  // namespace IE::Core
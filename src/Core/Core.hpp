#pragma once

#include "Core/EngineModule/Engine.hpp"
#include "Core/EngineModule/Window.hpp"
#include "Core/FileSystemModule/IEFileSystem.hpp"
#include "Core/LogModule/IELogger.hpp"

#include <mutex>
#include <unordered_map>

#define ILLUMINATION_ENGINE_CORE_LOGGER_NAME  "Illumination Engine"
#define ILLUMINATION_ENGINE_CORE_LOG_FILENAME "logs/IlluminationEngine.log"

class GLFWwindow;

namespace IE::Core {
class Core {
public:
    IE::Core::Logger logger{
      ILLUMINATION_ENGINE_CORE_LOGGER_NAME,
      ILLUMINATION_ENGINE_CORE_LOG_FILENAME,
      IE::Core::Logger::ILLUMINATION_ENGINE_LOG_TO_FILE | IE::Core::Logger::ILLUMINATION_ENGINE_LOG_TO_STDOUT};
    std::mutex                                          m_enginesMutex;
    std::unordered_map<std::string, IE::Core::Engine *> m_engines;
    std::mutex                                          m_windowsMutex;
    std::unordered_map<GLFWwindow *, IE::Core::Window>  m_windows;
    // @todo ThreadPool m_threadPool; Should be added here when the thread pool is brought into the engine.
    IEFileSystem                                        m_filesystem{"res"};

    static IE::Core::Core &getInst();

} __attribute__((aligned(128)));
}  // namespace IE::Core
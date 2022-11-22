#include "Core.hpp"

#include <concepts>

IE::Core::Logger IE::Core::Core::m_logger{
  ILLUMINATION_ENGINE_CORE_LOGGER_NAME,
  ILLUMINATION_ENGINE_CORE_LOG_FILENAME,
  IE::Core::Logger::ILLUMINATION_ENGINE_LOG_TO_FILE | IE::Core::Logger::ILLUMINATION_ENGINE_LOG_TO_STDOUT};
std::mutex                                          IE::Core::Core::m_enginesMutex{};
std::unordered_map<std::string, IE::Core::Engine *> IE::Core::Core::m_engines{};
std::mutex                                          IE::Core::Core::m_windowsMutex{};
std::unordered_map<GLFWwindow *, IE::Core::Window>  IE::Core::Core::m_windows{};
IE::Core::ThreadPool                                IE::Core::Core::m_threadPool{};
IEFileSystem                                        IE::Core::Core::m_filesystem{"res"};

IE::Core::Core &IE::Core::Core::getInst() {
    static IE::Core::Core inst{};
    return inst;
}

IE::Core::Logger *IE::Core::Core::getLogger() {
    return &m_logger;
}

IEFileSystem *IE::Core::Core::getFileSystem() {
    return &m_filesystem;
}

IE::Core::ThreadPool *IE::Core::Core::getThreadPool() {
    return &m_threadPool;
}

IE::Core::Window *IE::Core::Core::getWindow(GLFWwindow *t_window) {
    std::unique_lock<std::mutex> lock(m_windowsMutex);
    auto                         window = m_windows.find(t_window);
    if (window != m_windows.end()) return &window->second;
    else
        m_logger.log(
          "Window '" + std::to_string((uint64_t) t_window) + "' does not exist!",
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
        );
    return nullptr;
}

IE::Core::Engine *IE::Core::Core::getEngine(std::string id) {
    std::unique_lock<std::mutex> lock(m_enginesMutex);
    return m_engines.at(id);
}
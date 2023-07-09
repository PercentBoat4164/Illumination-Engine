#include "Core.hpp"

#include <concepts>

IE::Core::FileSystem                                IE::Core::Core::m_filesystem{};
IE::Core::Logger                                    IE::Core::Core::m_logger{ILLUMINATION_ENGINE_CORE_LOGGER_NAME};
std::mutex                                          IE::Core::Core::m_enginesMutex{};
std::unordered_map<std::string, IE::Core::Engine *> IE::Core::Core::m_engines{};
std::mutex                                          IE::Core::Core::m_windowsMutex{};
std::unordered_map<GLFWwindow *, IE::Core::Window>  IE::Core::Core::m_windows{};
IE::Core::Threading::ThreadPool                     IE::Core::Core::m_threadPool{};

IE::Core::Logger *IE::Core::Core::getLogger() {
    return &m_logger;
}

IE::Core::FileSystem *IE::Core::Core::getFileSystem() {
    return &m_filesystem;
}

IE::Core::Threading::ThreadPool *IE::Core::Core::getThreadPool() {
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
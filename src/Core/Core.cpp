#include "Core.hpp"

#include "EngineModule/EventActionMapping.hpp"
#include "FileSystemModule/FileSystem.hpp"

IE::Core::Logger IE::Core::Core::m_logger{
  ILLUMINATION_ENGINE_CORE_LOGGER_NAME,
  ILLUMINATION_ENGINE_CORE_LOG_FILENAME,
  IE::Core::Logger::ILLUMINATION_ENGINE_LOG_TO_FILE | IE::Core::Logger::ILLUMINATION_ENGINE_LOG_TO_STDOUT};
std::mutex                                                         IE::Core::Core::m_enginesMutex{};
std::unordered_map<std::string, std::shared_ptr<IE::Core::Engine>> IE::Core::Core::m_engines{};
std::mutex                                                         IE::Core::Core::m_windowsMutex{};
IE::Core::Threading::ThreadPool *const IE::Core::Core::m_threadPool{new Threading::ThreadPool{}};
IE::Core::FileSystem *const            IE::Core::Core::m_filesystem{new FileSystem{}};
IE::Core::EventActionMapping *const    IE::Core::Core::m_eventActionMapping{new EventActionMapping{}};

IE::Core::Core &IE::Core::Core::getInst(const std::filesystem::path &t_path) {
    static IE::Core::Core inst{t_path};
    return inst;
}

IE::Core::Logger *IE::Core::Core::getLogger() {
    return &m_logger;
}

IE::Core::FileSystem *IE::Core::Core::getFileSystem() {
    return m_filesystem;
}

IE::Core::Threading::ThreadPool *IE::Core::Core::getThreadPool() {
    return m_threadPool;
}

IE::Core::EventActionMapping *IE::Core::Core::getEventActionMapping() {
    return m_eventActionMapping;
}

IE::Core::Core::Core(const std::filesystem::path &t_path) {
    m_filesystem->setBaseDirectory(t_path);
}

#include "Core.hpp"

#include "EngineModule/EventActionMapping.hpp"
#include "FileSystemModule/FileSystem.hpp"

IE::Core::Logger                                    IE::Core::detail::Core::m_logger{ILLUMINATION_ENGINE_CORE_LOGGER_NAME};
std::mutex       IE::Core::detail::Core::m_enginesMutex{};
std::unordered_map<std::string, std::shared_ptr<IE::Core::Engine>> IE::Core::detail::Core::m_engines{};
std::mutex                                                         IE::Core::detail::Core::m_windowsMutex{};
IE::Core::Threading::ThreadPool                                    IE::Core::detail::Core::m_threadPool{};
IE::Core::FileSystem                                               IE::Core::detail::Core::m_filesystem{};
IE::Core::EventActionMapping                                       IE::Core::detail::Core::m_eventActionMapping{};

IE::Core::detail::Core &IE::Core::detail::Core::getInst(const std::filesystem::path &t_path) {
    static IE::Core::detail::Core inst{t_path};
    return inst;
}

IE::Core::Logger &IE::Core::detail::Core::getLogger() {
    return m_logger;
}

IE::Core::FileSystem &IE::Core::detail::Core::getFileSystem() {
    return m_filesystem;
}

IE::Core::Threading::ThreadPool &IE::Core::detail::Core::getThreadPool() {
    return m_threadPool;
}

IE::Core::EventActionMapping &IE::Core::detail::Core::getEventActionMapping() {
    return m_eventActionMapping;
}

IE::Core::detail::Core::Core(const std::filesystem::path &t_path) {
    m_filesystem.setBaseDirectory(t_path);
}

void IE::Core::init(const std::filesystem::path &t_path) {
    IE::Core::detail::Core::getInst(t_path);
}

IE::Core::Logger &IE::Core::getLogger() {
    return IE::Core::detail::Core::getLogger();
}

IE::Core::FileSystem &IE::Core::getFileSystem() {
    return IE::Core::detail::Core::getFileSystem();
}

IE::Core::Threading::ThreadPool &IE::Core::getThreadPool() {
    return IE::Core::detail::Core::getThreadPool();
}

IE::Core::EventActionMapping &IE::Core::getEventActionMapping() {
    return IE::Core::detail::Core::getEventActionMapping();
}

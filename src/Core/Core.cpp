#include "Core.hpp"

#include <concepts>

IE::Core::Logger                                    IE::Core::Core::m_logger{ILLUMINATION_ENGINE_CORE_LOGGER_NAME};
std::mutex                                          IE::Core::Core::m_enginesMutex{};
std::unordered_map<std::string, IE::Core::Engine *> IE::Core::Core::m_engines{};
IE::Core::Threading::ThreadPool                     IE::Core::Core::m_threadPool{};
IE::Core::FileSystem                                IE::Core::Core::m_filesystem{};
IE::Core::AssetManager                              IE::Core::Core::m_assetManager{m_filesystem};
IE::Core::InputHandler IE::Core::Core::m_inputHandler{};

IE::Core::Core &IE::Core::Core::getInst(const std::filesystem::path &t_path) {
    static IE::Core::Core inst{t_path};
    return inst;
}

IE::Core::Logger &IE::Core::Core::getLogger() {
    return m_logger;
}

IE::Core::FileSystem &IE::Core::Core::getFileSystem() {
    return m_filesystem;
}

IE::Core::Threading::ThreadPool &IE::Core::Core::getThreadPool() {
    return m_threadPool;
}

IE::Core::AssetManager &IE::Core::Core::getAssetManager() {
    return m_assetManager;
}

IE::Core::InputHandler &IE::Core::Core::getInputHandler() {
    return m_inputHandler;
}

IE::Core::Engine *IE::Core::Core::getEngine(std::string id) {
    std::unique_lock<std::mutex> lock(m_enginesMutex);
    return m_engines.at(id);
}
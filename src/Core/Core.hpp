#pragma once

#include "Core/LogModule/Logger.hpp"
#include "Core/ThreadingModule/ThreadPool.hpp"

#include <filesystem>
#include <mutex>
#include <unordered_map>

#define ILLUMINATION_ENGINE_CORE_LOGGER_NAME "Illumination Engine"

struct GLFWwindow;

namespace IE::Core {
class FileSystem;
class Engine;
class EventActionMapping;

class Core final {
public:
    static IE::Core::Core &getInst(const std::filesystem::path &t_path = "");

    template<typename T, typename... Args>
        requires std::derived_from<T, IE::Core::Engine>

    static std::shared_ptr<T> createEngine(std::string id, Args... args) {
        std::unique_lock<std::mutex> lock(m_enginesMutex);
        if (m_engines.find(id) != m_engines.end())
            m_logger.log(
              "Engine '" + id + "' already exists!",
              IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
            );
        std::shared_ptr<T> engine = std::make_shared<T>(id, args...);
        m_engines[id] = engine;
        return engine;
    }

    template<typename T>
        requires std::derived_from<T, IE::Core::Engine>

    static std::shared_ptr<T> getEngine(const std::string &id) {
        std::unique_lock<std::mutex> lock(m_enginesMutex);
        return std::static_pointer_cast<T>(m_engines.at(id));
    }

    static IE::Core::Logger                *getLogger();
    static IE::Core::FileSystem            *getFileSystem();
    static IE::Core::Threading::ThreadPool *getThreadPool();
    static IE::Core::EventActionMapping    *getEventActionMapping();

private:
    static IE::Core::Logger                                                   m_logger;
    static std::mutex                                                         m_enginesMutex;
    static std::unordered_map<std::string, std::shared_ptr<IE::Core::Engine>> m_engines;
    static std::mutex                                                         m_windowsMutex;
    static IE::Core::Threading::ThreadPool *const                             m_threadPool;
    static IE::Core::EventActionMapping *const                                m_eventActionMapping;
    static IE::Core::FileSystem *const                                        m_filesystem;

    explicit Core(const std::filesystem::path &t_path);

} __attribute__((aligned(128)));
}  // namespace IE::Core
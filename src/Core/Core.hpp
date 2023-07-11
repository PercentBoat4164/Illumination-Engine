#pragma once

#include "LogModule/Logger.hpp"

// Log module first because it depends on nothing.
#include "AssetModule/Aspect.hpp"
#include "AssetModule/Asset.hpp"
#include "FileSystemModule/FileSystem.hpp"
#include "ThreadingModule/ThreadPool.hpp"

#include <filesystem>
#include <mutex>
#include <unordered_map>

#define ILLUMINATION_ENGINE_CORE_LOGGER_NAME "Illumination Engine"

struct GLFWwindow;

namespace IE::Core {
class FileSystem;
class Engine;
class EventActionMapping;

namespace detail {
class Core final {
public:
    template<typename T, typename... Args>
        requires std::derived_from<T, IE::Core::Engine>

    static IE::Core::Threading::Task<std::shared_ptr<T>> createEngine(const std::string &id, Args... args) {
        std::unique_lock<std::mutex> lock(m_enginesMutex);
        if (m_engines.find(id) != m_engines.end())
            m_logger.log(
              "Engine '" + id + "' already exists!",
              IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
            );
        std::shared_ptr<IE::Core::Threading::Task<std::shared_ptr<T>>> engine = getThreadPool().submit(T::Factory(id, args...));
        co_await getThreadPool().resumeAfter(engine);
        m_engines[id]             = engine->value();
        co_return engine->value();
    }

    template<typename T>
        requires std::derived_from<T, IE::Core::Engine>

    static std::shared_ptr<T> getEngine(const std::string &id) {
        std::unique_lock<std::mutex> lock(m_enginesMutex);
        return std::static_pointer_cast<T>(m_engines.at(id));
    }

    static IE::Core::Logger                &getLogger();
    static IE::Core::FileSystem            &getFileSystem();
    static IE::Core::Threading::ThreadPool &getThreadPool();
    static IE::Core::EventActionMapping    &getEventActionMapping();

private:
    static IE::Core::Logger                                                   m_logger;
    static std::mutex                                                         m_enginesMutex;
    static std::unordered_map<std::string, std::shared_ptr<IE::Core::Engine>> m_engines;
    static std::mutex                                                         m_windowsMutex;
    static Threading::ThreadPool                                              m_threadPool;
    static IE::Core::EventActionMapping                                       m_eventActionMapping;
    static FileSystem                                                         m_filesystem;
};
}  // namespace detail

IE::Core::Logger                &getLogger();
IE::Core::FileSystem            &getFileSystem();
IE::Core::Threading::ThreadPool &getThreadPool();
IE::Core::EventActionMapping    &getEventActionMapping();

template<typename T, typename... Args>
    requires std::derived_from<T, IE::Core::Engine>
static IE::Core::Threading::Task<std::shared_ptr<T>> createEngine(const std::string &t_id, Args... args) {
    auto e = IE::Core::getThreadPool().submit(IE::Core::detail::Core::createEngine<T>(t_id, args...));
    co_await IE::Core::getThreadPool().resumeAfter(e);
    co_return e->value();
}

template<typename T>
    requires std::derived_from<T, IE::Core::Engine>
static std::shared_ptr<T> getEngine(const std::string &t_id) {
    return IE::Core::detail::Core::getEngine<T>(t_id);
}
}  // namespace IE::Core

// Modules that depend on Core itself should go last.
#include "EngineModule/Engine.hpp"

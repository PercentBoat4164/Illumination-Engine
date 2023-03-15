#pragma once

#include "Core/EngineModule/Engine.hpp"
#include "Core/EngineModule/Window.hpp"
#include "Core/FileSystemModule/FileSystem.hpp"
#include "Core/LogModule/IELogger.hpp"
#include "Core/ThreadingModule/ThreadPool.hpp"

#include <SDL.h>
#include <SDL_vulkan.h>


#include <mutex>
#include <unordered_map>

#define ILLUMINATION_ENGINE_CORE_LOGGER_NAME  "Illumination Engine"
#define ILLUMINATION_ENGINE_CORE_LOG_FILENAME "logs/IlluminationEngine.log"

struct SDL_Window;

namespace IE::Core {
class Core final {
public:
    static IE::Core::Core &getInst(const std::filesystem::path &t_path = "");

    template<typename T, typename... Args>
        requires std::derived_from<T, IE::Core::Engine>

    static T *createEngine(std::string id, Args... args) {
        std::unique_lock<std::mutex> lock(m_enginesMutex);
        if (m_engines.find(id) != m_engines.end())
            m_logger.log(
              "Engine '" + id + "' already exists!",
              IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
            );
        m_engines[id] = new T(args...);
        return static_cast<T *>(m_engines[id]);
    }

    template<typename T>
        requires std::derived_from<T, IE::Core::Engine>

    static T *getEngine(std::string id) {
        std::unique_lock<std::mutex> lock(m_enginesMutex);
        return static_cast<T *>(m_engines.at(id));
    }

    static IE::Core::Engine *getEngine(std::string id);

    template<typename... Args>
    static void registerWindow(SDL_Window *t_window, Args... args) {
        std::unique_lock<std::mutex> lock(m_windowsMutex);
        if (m_windows.find(t_window) != m_windows.end())
            m_logger.log(
              "Window '" + std::to_string((uint64_t) t_window) + "' already exists!",
              IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
            );
        m_windows[t_window] = Window(args...);
    }

    static IE::Core::Window *getWindow(SDL_Window *t_window);

    static Logger                *getLogger();
    static FileSystem            *getFileSystem();
    static Threading::ThreadPool *getThreadPool();

private:
    static IE::Core::Logger                                    m_logger;
    static std::mutex                                          m_enginesMutex;
    static std::unordered_map<std::string, IE::Core::Engine *> m_engines;
    static std::mutex                                          m_windowsMutex;
    static std::unordered_map<SDL_Window *, IE::Core::Window>  m_windows;
    static Threading::ThreadPool                               m_threadPool;
    static FileSystem                                          m_filesystem;

    Core(const std::filesystem::path &t_path) {
        m_filesystem.setBaseDirectory(t_path);
        SDL_Init(SDL_INIT_EVERYTHING);
    }
};
}
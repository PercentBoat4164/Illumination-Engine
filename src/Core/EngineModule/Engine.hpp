#pragma once

#include "Core/AssetModule/Aspect.hpp"

#include <future>
#include <memory>
#include <unordered_map>

namespace IE::Core {
class File;

class Engine {
    using AspectType = IE::Core::Aspect;

protected:
    std::mutex                                               m_aspectsMutex;
    std::unordered_map<std::string, std::shared_ptr<Aspect>> m_aspects;
    std::mutex                                               m_jobsMutex;
    std::vector<std::future<void>>                           m_jobs;

    template<typename T>

    requires std::derived_from<T, IE::Core::Aspect> std::shared_ptr<T>
    _createAspect(const std::string &t_id, IE::Core::File *t_resource, IE::Core::Engine *downCastedSelf) {
        std::shared_ptr<T> aspect = _getAspect<T>(t_id);
        if (aspect == nullptr) {
            aspect = std::make_shared<T>(downCastedSelf, t_resource);
            std::unique_lock<std::mutex> lock(m_aspectsMutex);
            m_aspects[t_id] = aspect;
        }
        return aspect;
    }

    template<typename T>

    requires std::derived_from<T, IE::Core::Aspect> std::shared_ptr<T> _getAspect(const std::string &t_id) {
        std::unique_lock<std::mutex> lock(m_aspectsMutex);
        auto                         aspect = m_aspects.find(t_id);
        if (aspect != m_aspects.end()) return std::static_pointer_cast<T>(aspect->second);
        return nullptr;
    }

public:
    Engine() = default;

    Engine &operator=(const IE::Core::Engine &t_other);

    Engine &operator=(IE::Core::Engine &&t_other) noexcept;

    virtual ~Engine() = default;

    virtual std::shared_ptr<Engine> create() = 0;

    virtual bool update() = 0;

    virtual bool finish();
};
}  // namespace IE::Core
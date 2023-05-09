#pragma once

#include "Instance.hpp"

#include <string>
#include <vector>

#define GLM_FORCE_RADIANS

#include <algorithm>
#include <glm/glm.hpp>
#include <memory>

namespace IE::Core {
class Aspect;
class File;

class Asset {
private:
    template<typename... Args>
    void fillInstances(IE::Core::Instance *t_instance, Args... args) {
        addInstance(t_instance);
        if constexpr (sizeof...(args) > 0) fillInstances(args...);
    }

    template<typename... Args>
    void fillInstances(IE::Core::Aspect *t_aspect, Args... args) {
        addInstance(t_aspect);
        if constexpr (sizeof...(args) > 0) fillInstances(args...);
    }

    template<typename... Args>
    void fillInstances(std::string t_aspectID, Args... args) {
        addInstance(t_aspectID);
        if constexpr (sizeof...(args) > 0) fillInstances(args...);
    }

public:
    // Things that are shared among all m_instances of an asset
    glm::vec3                         m_position{0.0, 0.0, 0.0};
    glm::vec3                         m_rotation{0.0, 0.0, 0.0};
    glm::vec3                         m_scale{1.0, 1.0, 1.0};
    IE::Core::File                   *m_resourceFile{};
    std::vector<IE::Core::Instance *> m_instances;

    void addInstance(IE::Core::Instance *t_instance);

    void addInstance(IE::Core::Aspect *t_aspect);

    void addInstance(std::string t_aspectID);

    template<typename... Args>
    Asset(IE::Core::File *t_resourceFile, Args... args) : m_resourceFile(t_resourceFile) {
        if constexpr (sizeof...(args) > 0) fillInstances(args...);
    }

    Asset(IE::Core::File *t_resourceFile, std::vector<IE::Core::Instance *> t_instances);

    Asset(IE::Core::File *t_resourceFile, std::vector<IE::Core::Aspect *> t_aspects);

    ~Asset();
};
}  // namespace IE::Core

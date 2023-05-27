#pragma once

#include "Instance.hpp"

#define GLM_FORCE_RADIANS

#include <algorithm>
#include <glm/glm.hpp>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace IE::Core {
class Aspect;
class File;

class Asset : public std::enable_shared_from_this<Asset> {
private:
    template<typename... Args>
    void fillInstances(std::shared_ptr<IE::Core::Aspect> t_aspect, Args... args) {
        addInstance(t_aspect);
        if constexpr (sizeof...(args) > 0) return fillInstances(args...);
    }

    template<typename... Args>
    void fillInstances(const std::string &t_aspectID, Args... args) {
        addInstance(t_aspectID);
        if constexpr (sizeof...(args) > 0) return fillInstances(args...);
    }

    explicit Asset(const std::string &t_id, IE::Core::File *t_resourceFile);

public:
    // Things that are shared among all m_instances of an asset
    glm::vec3                                        m_position{0.0, 0.0, 0.0};
    glm::vec3                                        m_rotation{0.0, 0.0, 0.0};
    glm::vec3                                        m_scale{1.0, 1.0, 1.0};
    IE::Core::File                                  *m_resourceFile{};
    std::string                                      m_id;
    std::vector<std::shared_ptr<IE::Core::Instance>> m_instances;
    std::mutex                                       m_instancesMutex;

    void addInstance(std::shared_ptr<IE::Core::Aspect> t_aspect);

    void addInstance(const std::string &t_aspectID);

    template<typename... Args>
    static std::shared_ptr<Asset>
    Factory(const std::string &t_id, IE::Core::File *t_resourceFile, Args... t_args) {
        auto asset = std::shared_ptr<Asset>(new Asset(t_id, t_resourceFile));
        if constexpr (sizeof...(t_args) > 0) asset->fillInstances(t_args...);
        return asset;
    }

    static std::shared_ptr<Asset> Factory(
      const std::string                                    &t_id,
      IE::Core::File                                       *t_resourceFile,
      const std::vector<std::shared_ptr<IE::Core::Aspect>> &t_aspects
    );
};
}  // namespace IE::Core

#include "Instance.hpp"

#include "Aspect.hpp"
#include "Asset.hpp"

#include <mutex>

IE::Core::Instance::Instance(const std::shared_ptr<IE::Core::Asset> &t_asset, const std::shared_ptr<IE::Core::Aspect> &t_aspect) :
        m_asset(t_asset),
        m_aspect(t_aspect) {
}

std::shared_ptr<IE::Core::Instance>
IE::Core::Instance::Factory(const std::shared_ptr<IE::Core::Asset> &t_asset, const std::shared_ptr<IE::Core::Aspect> &t_aspect) {
    std::shared_ptr<Instance> instance{std::shared_ptr<Instance>(new Instance(t_asset, t_aspect))};
    {
        std::lock_guard<std::mutex> lock(t_asset->m_instancesMutex);
        t_asset->m_instances.push_back(instance);
    }
    {
        std::lock_guard<std::mutex> lock(t_aspect->m_instancesMutex);
        t_aspect->m_instances.push_back(instance);
    }
    return instance;
}

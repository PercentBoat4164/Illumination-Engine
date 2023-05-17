#include "Asset.hpp"

#include "Core/Core.hpp"

void IE::Core::Asset::addInstance(std::shared_ptr<IE::Core::Aspect> t_aspect) {
    Instance::Factory(shared_from_this(), t_aspect);
}

void IE::Core::Asset::addInstance(const std::string &t_aspectID) {
    Instance::Factory(shared_from_this(), IE::Core::Core::getAssetManager().getAspect(t_aspectID));
}

IE::Core::Asset::Asset(const std::string &t_id, IE::Core::File *t_resourceFile) :
        m_resourceFile(t_resourceFile),
        m_id(t_id) {
}

std::shared_ptr<IE::Core::Asset> IE::Core::Asset::Factory(
  const std::string                                    &t_id,
  IE::Core::File                                       *t_resourceFile,
  const std::vector<std::shared_ptr<IE::Core::Aspect>> &t_aspects
) {
    auto asset = std::shared_ptr<Asset>(new Asset(t_id, t_resourceFile));
    for (const std::shared_ptr<IE::Core::Aspect> &aspect : t_aspects) Instance::Factory(asset, aspect);
    return asset;
}

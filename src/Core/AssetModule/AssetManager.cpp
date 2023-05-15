#include "AssetManager.hpp"

IE::Core::AssetManager::AssetManager(IE::Core::FileSystem &t_filesystem) : m_filesystem(t_filesystem) {
}

void IE::Core::AssetManager::removeAsset(const std::string &t_id) {
    std::lock_guard<std::mutex> lock(m_assetsMutex);
    m_assets.erase(t_id);
}

void IE::Core::AssetManager::removeAsset(IE::Core::Asset &t_asset) {
    std::lock_guard<std::mutex> lock(m_assetsMutex);
    m_assets.erase(t_asset.m_id);
}

void IE::Core::AssetManager::removeAspect(const std::string &t_id) {
    std::lock_guard<std::mutex> lock(m_aspectsMutex);
    m_aspects.erase(t_id);
}

void IE::Core::AssetManager::removeAspect(IE::Core::Aspect &t_aspect) {
    std::lock_guard<std::mutex> lock(m_aspectsMutex);
    m_aspects.erase(t_aspect.m_id);
}

std::shared_ptr<IE::Core::Aspect> IE::Core::AssetManager::getAspect(const std::string &t_id) {
    std::lock_guard<std::mutex> lock(m_aspectsMutex);
    auto iterator = m_aspects.find(t_id);
    if (iterator == m_aspects.end()) return nullptr;
    return iterator->second;
}

std::shared_ptr<IE::Core::Asset> IE::Core::AssetManager::getAsset(const std::string &t_id) {
    std::lock_guard<std::mutex> lock(m_assetsMutex);
    auto iterator = m_assets.find(t_id);
    if (iterator == m_assets.end()) return nullptr;
    return iterator->second;
}

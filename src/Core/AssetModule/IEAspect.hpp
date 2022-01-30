#pragma once

#include <vector>

class IEAsset;

class IEAspect {
public:
    // This is a requirement of an Aspect
    std::vector<IEAsset*> associatedAssets{};
};
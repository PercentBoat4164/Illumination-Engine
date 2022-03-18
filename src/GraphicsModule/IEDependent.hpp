#pragma once

class IEDependency;

#include <vector>

class IEDependent {
    std::vector<IEDependency*> dependencies{};

public:
    void addDependency(IEDependency *dependency);

    void addDependencies(const std::vector<IEDependency *>& dependencies);

    bool isDependentOn(IEDependency *dependency);

    void removeDependency(IEDependency *dependency);

    bool hasNoDependencies();
};
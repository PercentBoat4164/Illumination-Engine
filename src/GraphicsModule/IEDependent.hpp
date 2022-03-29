#pragma once

class IEDependency;

#include <vector>

class IEDependent {

protected:
    std::vector<IEDependency*> dependencies{};
public:
    void addDependency(IEDependency *dependency);

    void addDependencies(const std::vector<IEDependency *>& newDependencies);

    bool isDependentOn(IEDependency *dependency);

    void removeDependency(IEDependency *dependency);

    bool hasNoDependencies();
};
#pragma once

class IEDependency;

#include <vector>

class IEDependent {

protected:
    std::vector<IEDependency*> dependencies{};

public:
    virtual ~IEDependent() = default;

    void addDependency(IEDependency *dependency);

    void addDependencies(const std::vector<IEDependency *>& newDependencies);

    bool isDependentOn(IEDependency *dependency);

    void removeDependency(IEDependency *dependency);

    void removeAllDependencies();

    bool hasNoDependencies();

    virtual void wait() = 0;
};
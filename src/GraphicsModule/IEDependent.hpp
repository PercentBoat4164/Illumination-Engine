#pragma once

class IEDependency;

#include <vector>

class IEDependent {

protected:
public:

    std::vector<IEDependency*> dependencies{};

    virtual void destroy();

    virtual ~IEDependent();

    void addDependency(IEDependency *dependency);

    void addDependencies(const std::vector<IEDependency *>& newDependencies);

    bool isDependentOn(IEDependency *dependency);

    void removeDependency(IEDependency *dependency);

    void removeAllDependencies();

    bool hasNoDependencies() const;

    virtual void wait() = 0;
};
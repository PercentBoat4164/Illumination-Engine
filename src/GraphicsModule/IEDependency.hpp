#pragma once

class IEDependent;

#include <vector>

class IEDependency {
protected:
    std::vector<IEDependent*> dependents{};

public:
    void addDependent(IEDependent *dependent);

    bool isDependencyOf(IEDependent *dependent);

    void removeDependent(IEDependent *dependent);

    void removeAllDependents();

    bool hasNoDependents();

    virtual ~IEDependency() = 0;

    void wait();
};

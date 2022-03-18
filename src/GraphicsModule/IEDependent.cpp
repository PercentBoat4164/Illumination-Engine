#include <algorithm>
#include "IEDependent.hpp"

#include "IEDependency.hpp"

void IEDependent::addDependency(IEDependency *dependency) {
    if (!isDependentOn(dependency)) {
        dependencies.push_back(dependency);
    }
    if (!dependency->isDependencyOf(this)) {
        dependency->addDependent(this);
    }
}

void IEDependent::addDependencies(const std::vector<IEDependency *>& dependencies) {
    for (IEDependency *dependency : dependencies) {
        addDependency(dependency);
    }
}

bool IEDependent::isDependentOn(IEDependency *dependency) {
    return std::any_of(dependencies.begin(), dependencies.end(), [dependency](IEDependency *thisDependency) { return thisDependency == dependency; });
}

bool IEDependent::hasNoDependencies() {
    return dependencies.empty();
}

void IEDependent::removeDependency(IEDependency *dependency) {
    dependencies.erase(std::find(dependencies.begin(), dependencies.end(), dependency));
    if (dependency->hasNoDependents()) {
        dependency->~IEDependency();
    }
}
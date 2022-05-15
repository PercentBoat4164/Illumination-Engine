#include <algorithm>
#include "IEDependent.hpp"

#include "IEDependency.hpp"

void IEDependent::addDependency(const std::shared_ptr<IEDependency>& newDependency) {
    dependencies.push_back(newDependency);
}

void IEDependent::addDependencies(const std::vector<std::shared_ptr<IEDependency>> &newDependencies) {
    dependencies.insert(dependencies.end(), newDependencies.begin(), newDependencies.end());
}

void IEDependent::removeDependency(const std::shared_ptr<IEDependency> &oldDependency) {
    dependencies.erase(std::find_if(dependencies.begin(), dependencies.end(), [&](const std::shared_ptr<IEDependency> &item) {
        return item == oldDependency;
    }));
}

void IEDependent::removeDependencies(const std::vector<std::shared_ptr<IEDependency>> &oldDependencies) {
    for (const std::shared_ptr<IEDependency> &oldDependency: oldDependencies) {
        removeDependency(oldDependency);
    }
}

uint32_t IEDependent::dependencyCount() {
    return dependencies.size();
}

void IEDependent::clearAllDependencies() {
    dependencies.clear();
}

bool IEDependent::canBeDestroyed(IEDependency *, bool) {
    return false;
}

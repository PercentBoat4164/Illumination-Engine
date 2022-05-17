#include <algorithm>
#include "IEDependency.hpp"

#include "IEDependent.hpp"

#include "Image/IEImage.hpp"
#include "Buffer/IEBuffer.hpp"
#include "IEPipeline.hpp"
#include "IEDescriptorSet.hpp"
#include "IERenderPass.hpp"

void IEDependency::addDependent(IEDependent *newDependent) {
    dependents.push_back(newDependent);
}

void IEDependency::addDependent(IEDependent &newDependent) {
    dependents.push_back(&newDependent);
}

void IEDependency::addDependents(const std::vector<IEDependent *> &newDependents) {
    dependents.insert(dependents.begin(), newDependents.begin(), newDependents.end());
}

void IEDependency::addDependents(std::vector<IEDependent> &newDependents) {
    dependents.reserve(dependents.size() + newDependents.size());
    for (IEDependent &dependent : newDependents) {
        addDependent(dependent);
    }
}

void IEDependency::removeDependent(IEDependent *oldDependent) {
    dependents.erase(std::find_if(dependents.begin(), dependents.end(), [&](IEDependent *item) {
        return item == oldDependent;
    }));
}

void IEDependency::removeDependent(IEDependent &oldDependent) {
    dependents.erase(std::find_if(dependents.begin(), dependents.end(), [&](IEDependent *item) {
        return item == &oldDependent;
    }));
}

void IEDependency::removeDependents(const std::vector<IEDependent *> &oldDependents) {
    for (IEDependent *dependent : oldDependents) {
        removeDependent(dependent);
    }
}

void IEDependency::removeDependents(std::vector<IEDependent> &oldDependents) {
    for (IEDependent &dependent : oldDependents) {
        removeDependent(dependent);
    }
}

void IEDependency::invalidateDependents() {
    for (IEDependent *dependent : dependents) {
        dependent->invalidate();
    }
}

bool IEDependency::canBeDestroyed(bool force) {
    bool result = true;
    for (IEDependent *dependent : dependents) {
        result |= dependent->canBeDestroyed(std::shared_ptr<IEDependency>(this), force);
    }
    return result;
}

#include <algorithm>
#include <memory>
#include <iostream>
#include "IEDependent.hpp"
#include "Buffer/IEBuffer.hpp"
#include "GraphicsModule/RenderPass/IEFramebuffer.hpp"

#include "IEDependency.hpp"

void IEDependent::addDependency(const std::shared_ptr<IEDependency> &newDependency) {
	if (!isDependentOn(newDependency)) {
		dependencies.push_back(newDependency);
	}
}

void IEDependent::addDependencies(const std::vector<std::shared_ptr<IEDependency>> &newDependencies) {
	for (const std::shared_ptr<IEDependency> &newDependency: newDependencies) {
		addDependency(newDependency);
	}
}

void IEDependent::removeDependency(const std::shared_ptr<IEDependency> &oldDependency) {
	dependencies.erase(std::find_if(dependencies.begin(), dependencies.end(), [&](const std::shared_ptr<IEDependency> &thisDependency) {
		return thisDependency == oldDependency;
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

bool IEDependent::canBeDestroyed(const std::shared_ptr<IEDependency> &, bool) {
	return false;
}

bool IEDependent::canBeDestroyed(IEDependency *, bool) {
	return false;
}

bool IEDependent::isDependentOn(const std::shared_ptr<IEDependency> &dependency) {
	return std::any_of(dependencies.begin(), dependencies.end(),
					   [&](const std::shared_ptr<IEDependency> &item) { return item.get() == dependency.get(); });
}
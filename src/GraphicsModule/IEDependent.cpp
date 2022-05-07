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

void IEDependent::addDependencies(const std::vector<IEDependency *> &newDependencies) {
	for (IEDependency *dependency: newDependencies) {
		addDependency(dependency);
	}
}

bool IEDependent::isDependentOn(IEDependency *dependency) {
	return std::any_of(dependencies.begin(), dependencies.end(), [dependency](IEDependency *thisDependency) { return thisDependency == dependency; });
}

bool IEDependent::hasNoDependencies() const {
	return dependencies.empty();
}

void IEDependent::removeDependency(IEDependency *dependency) {
	dependencies.erase(std::remove(dependencies.begin(), dependencies.end(), dependency), dependencies.end());
	if (dependency->isDependencyOf(this)) {
		dependency->removeDependent(this);
	}
}

void IEDependent::removeAllDependencies() {
	for (IEDependency *dependency: dependencies) {
		dependency->removeDependent(this);
	}
	dependencies.clear();
}

void IEDependent::destroy() {
	removeAllDependencies();
}

IEDependent::~IEDependent() {
	destroy();
}

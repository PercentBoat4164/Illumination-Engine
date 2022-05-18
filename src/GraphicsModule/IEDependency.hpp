#pragma once

class IEDependent;

class IEImage;

class IEFramebuffer;

class IEBuffer;

class IEPipeline;

class IEDescriptorSet;

class IERenderPass;

#include <vector>
#include <vulkan/vulkan.h>
#include <string>
#include <memory>


class IEDependency {
private:
    std::vector<IEDependent *> dependents{};

public:
    void addDependent(IEDependent *);

    void addDependent(IEDependent &);

    void addDependents(const std::vector<IEDependent *> &);

    void addDependents(std::vector<IEDependent> &);

    void removeDependent(IEDependent *);

    void removeDependent(IEDependent &);

    void removeDependents(const std::vector<IEDependent *> &);

    void removeDependents(std::vector<IEDependent> &);

    void invalidateDependents();

    bool canBeDestroyed(bool=false);
	
	virtual ~IEDependency() = default;
};
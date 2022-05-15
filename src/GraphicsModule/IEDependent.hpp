#pragma once

class IEDependency;

#include <vector>
#include <memory>

class IEDependent {
private:
    std::vector<std::shared_ptr<IEDependency>> dependencies{};

protected:
    void addDependency(const std::shared_ptr<IEDependency>&);

    void addDependencies(const std::vector<std::shared_ptr<IEDependency>> &);

    void removeDependency(const std::shared_ptr<IEDependency> &);

    void removeDependencies(const std::vector<std::shared_ptr<IEDependency>> &oldDependencies);

public:
    void clearAllDependencies();

    uint32_t dependencyCount();

    virtual bool canBeDestroyed(IEDependency *, bool);;

    virtual void freeDependencies() {};

    virtual void invalidate() {};
};
#pragma once

#include <memory>

class MultipleInheritableEnableSharedFromThis :
        public std::enable_shared_from_this<MultipleInheritableEnableSharedFromThis> {
    virtual void MAKE_POLYMORPHIC() {
    }
};

template<class T>
class inheritable_enable_shared_from_this : virtual public MultipleInheritableEnableSharedFromThis {
public:
    std::shared_ptr<T> shared_from_this() {
        return std::dynamic_pointer_cast<T>(MultipleInheritableEnableSharedFromThis::shared_from_this());
    }

    template<class Down>
    std::shared_ptr<Down> downcasted_shared_from_this() {
        return std::dynamic_pointer_cast<Down>(MultipleInheritableEnableSharedFromThis::shared_from_this());
    }
};
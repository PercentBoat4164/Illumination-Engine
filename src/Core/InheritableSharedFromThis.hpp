#pragma once

#include <memory>

namespace IE::Core {
namespace detail {
struct MultipleInheritableEnableSharedFromThis :
        public std::enable_shared_from_this<MultipleInheritableEnableSharedFromThis> {
    virtual ~MultipleInheritableEnableSharedFromThis(){};
};
}  // namespace detail

template<class T>
class InheritableSharedFromThis : virtual public IE::Core::detail::MultipleInheritableEnableSharedFromThis {
public:
    std::shared_ptr<T> shared_from_this() {
        return std::dynamic_pointer_cast<T>(
          IE::Core::detail::MultipleInheritableEnableSharedFromThis::shared_from_this()
        );
    }

    template<class DowncastType>
    std::shared_ptr<DowncastType> shared_from_this() {
        return std::dynamic_pointer_cast<DowncastType>(
          IE::Core::detail::MultipleInheritableEnableSharedFromThis::shared_from_this()
        );
    }

    std::weak_ptr<T> weak_from_this() {
        return std::dynamic_pointer_cast<T>(
          IE::Core::detail::MultipleInheritableEnableSharedFromThis::shared_from_this()
        );
    }

    template<class DowncastType>
    std::weak_ptr<DowncastType> weak_from_this() {
        return std::dynamic_pointer_cast<DowncastType>(
          IE::Core::detail::MultipleInheritableEnableSharedFromThis::shared_from_this()
        );
    }
};
}  // namespace IE::Core
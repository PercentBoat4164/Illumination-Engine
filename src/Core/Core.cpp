#include "Core.hpp"

IE::Core::Core &IE::Core::Core::getInst() {
    static IE::Core::Core inst{};
    return inst;
}

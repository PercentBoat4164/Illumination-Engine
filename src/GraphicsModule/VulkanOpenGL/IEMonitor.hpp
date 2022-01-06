#pragma once

#include <cstdint>

/**@todo Move this class to the GUI module when it comes into existence.*/

class IEMonitor {
    uint16_t refreshRate{};
    uint16_t resolution[2]{};
    uint16_t currentResolution[2]{};
    uint16_t position[2]{};
};
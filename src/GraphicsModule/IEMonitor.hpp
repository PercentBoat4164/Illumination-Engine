#pragma once

/* Include classes used as attributes or _function arguments. */
// System dependencies
#include <cstdint>

class IEMonitor {
	uint16_t refreshRate{};
	uint16_t resolution[2]{};
	uint16_t currentResolution[2]{};
	uint16_t position[2]{};
};
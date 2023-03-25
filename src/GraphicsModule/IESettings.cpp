/* Include this file's header. */
#include "IESettings.hpp"

void IESettings::updateWindowDimensions() {
    int totalDisplays = SDL_GetNumVideoDisplays();
    for (int i = 0; i < totalDisplays; i++) SDL_GetDisplayUsableBounds(i, &displayDimensions);
    currentResolution  = {displayDimensions.w / 2, displayDimensions.h / 2};
}


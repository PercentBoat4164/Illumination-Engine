#pragma once

namespace IE::Graphics {
class Settings {
public:
    using ShadowMethods = enum {
        IE_GRAPHICS_SETTINGS_SHADOW_METHOD_NONE                               = 0x0,
        IE_GRAPHICS_SETTINGS_SHADOW_METHOD_SHADOW_MAPPING                     = 0x1,
        IE_GRAPHICS_SETTINGS_SHADOW_METHOD_CASCADED_SHADOW_MAPPING            = 0x2,
        IE_GRAPHICS_SETTINGS_SHADOW_METHOD_SAMPLE_DISTRIBUTION_SHADOW_MAPPING = 0x3,
    };

    using ShadowFiltering = enum {
        IE_GRAPHICS_SETTINGS_SHADOW_FILTER_NONE = 0x0,
        IE_GRAPHICS_SETTINGS_SHADOW_FILTER_PCF  = 0x1,
        IE_GRAPHICS_SETTINGS_SHADOW_FILTER_PCSS = 0x2,
    };

    ShadowMethods   shadowMethods{IE_GRAPHICS_SETTINGS_SHADOW_METHOD_NONE};
    ShadowFiltering shadowFiltering{IE_GRAPHICS_SETTINGS_SHADOW_FILTER_NONE};
};
}  // namespace IE::Graphics
#pragma once

namespace IE::Graphics {
class Settings {
public:
    enum ShadowMethods {
        IE_GRAPHICS_SETTINGS_SHADOW_METHOD_NONE                               = 0x0,
        IE_GRAPHICS_SETTINGS_SHADOW_METHOD_SHADOW_MAPPING                     = 0x1,
        IE_GRAPHICS_SETTINGS_SHADOW_METHOD_CASCADED_SHADOW_MAPPING            = 0x2,
        IE_GRAPHICS_SETTINGS_SHADOW_METHOD_SAMPLE_DISTRIBUTION_SHADOW_MAPPING = 0x3,
    };

    enum ShadowFilters {
        IE_GRAPHICS_SETTINGS_SHADOW_FILTER_NONE = 0x0,
        IE_GRAPHICS_SETTINGS_SHADOW_FILTER_PCF  = 0x1,
        IE_GRAPHICS_SETTINGS_SHADOW_FILTER_PCSS = 0x2,
    };

    enum VSyncMethods {
        IE_GRAPHICS_SETTINGS_VSYNC_IMMEDIATE = 0x0,
    };

    enum MSAAQuality {
        IE_GRAPHICS_SETTINGS_MSAA_QUALITY_1  = 0x1,
        IE_GRAPHICS_SETTINGS_MSAA_QUALITY_2  = 0x2,
        IE_GRAPHICS_SETTINGS_MSAA_QUALITY_4  = 0x4,
        IE_GRAPHICS_SETTINGS_MSAA_QUALITY_8  = 0x8,
        IE_GRAPHICS_SETTINGS_MSAA_QUALITY_16 = 0x16,
        IE_GRAPHICS_SETTINGS_MSAA_QUALITY_32 = 0x32,
        IE_GRAPHICS_SETTINGS_MSAA_QUALITY_64 = 0x64,
    };

    /**
     * Format is given by:
     * 4x bytes specifying bits per channel.
     * 1x byte specifying the ordering of channels.
     *      Every two bits is another channel specification. The two bits index an array of RGBA.
     *      e.x. RGBA = 0x1B
     *           BRGA = 0x87
     * 1x byte specifying the data type.
     *      The byte indexes an array of: INT, UINT, FLOAT
     */
    enum ImageFormat : uint64_t {
        IE_GRAPHICS_IMAGE_FORMAT_RGBA8888_INT = 0x0000080808081B00U,
        IE_GRAPHICS_IMAGE_FORMAT_A24_FLOAT    = 0x0000000000181B02U,
    };

    ShadowMethods shadowMethod{IE_GRAPHICS_SETTINGS_SHADOW_METHOD_NONE};
    ShadowFilters shadowFilter{IE_GRAPHICS_SETTINGS_SHADOW_FILTER_NONE};
    VSyncMethods  vSyncMethod{IE_GRAPHICS_SETTINGS_VSYNC_IMMEDIATE};
    MSAAQuality   msaaQuality{IE_GRAPHICS_SETTINGS_MSAA_QUALITY_1};
    ImageFormat   presentFormat{};
};
}  // namespace IE::Graphics
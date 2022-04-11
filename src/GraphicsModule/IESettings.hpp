#pragma once

/* Define macros used throughout the file. */
#define ILLUMINATION_ENGINE_VERSION_MAJOR 0
#define ILLUMINATION_ENGINE_VERSION_MINOR 0
#define ILLUMINATION_ENGINE_VERSION_PATCH 0
#define ILLUMINATION_ENGINE_NAME "Illumination Engine"

/* Include classes used as attributes or function arguments. */
// Internal dependencies
#include "IEVersion.hpp"

#include "Core/LogModule/IELogger.hpp"

// External dependencies
#include <vulkan/vulkan.h>

// System dependencies
#include <string>
#include <array>


class IESettings {
public:
    IELogger logger{};
    bool rayTracing{false};
    std::string applicationName{"Illumination Engine"};
    IEVersion applicationVersion{0, 0, 1};
    IEVersion minimumVulkanVersion{1, 0, 0};
    IEVersion desiredVulkanVersion{1, 2, 0};
    uint8_t msaaSamples{VK_SAMPLE_COUNT_1_BIT};
    std::array<int, 2> defaultWindowResolution{800, 600};
    std::array<int, 2> windowPosition{0, 0};
    float anisotropicFilterLevel{16.0f};
    bool mipMapping{true};
    float mipMapLevel{0};
    bool fullscreen{false};
    int refreshRate{60};
    bool vSync{true};
    std::array<int, 2> resolution{defaultWindowResolution};
    double fov{90};
    double renderDistance{1000000};
    double mouseSensitivity{0.1};
    float movementSpeed{2.5};
};
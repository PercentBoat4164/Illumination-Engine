#pragma once

/* Define macros used throughout the file. */
#define ILLUMINATION_ENGINE_VERSION_MAJOR 0
#define ILLUMINATION_ENGINE_VERSION_MINOR 0
#define ILLUMINATION_ENGINE_VERSION_PATCH 0
#define ILLUMINATION_ENGINE_NAME          "Illumination Engine"

/* Include classes used as attributes or function arguments. */
// Internal dependencies
#include "Core/LogModule/IELogger.hpp"
#include "IEVersion.hpp"

// External dependencies
#define GLEW_IMPLEMENTATION
#include <GL/glew.h>


// System dependencies
#include <array>
#include <GLFW/glfw3.h>
#include <string>

class IESettings {
public:
    IESettings() {
        glfwInit();
        primaryMonitor       = glfwGetPrimaryMonitor();
        defaultResolution    = {800, 600};
        fullscreenResolution = {glfwGetVideoMode(primaryMonitor)->width, glfwGetVideoMode(primaryMonitor)->height};
        windowedResolution   = {defaultResolution};
        currentResolution    = fullscreen ? &fullscreenResolution : &windowedResolution;
        defaultPosition      = {
               (glfwGetVideoMode(primaryMonitor)->width - windowedResolution[0]) / 2,
               (glfwGetVideoMode(primaryMonitor)->height - windowedResolution[1]) / 2};
        fullscreenPosition = {0, 0};
        windowedPosition   = {defaultPosition};
        currentPosition    = fullscreen ? &fullscreenPosition : &windowedPosition;
    }

    IE::Core::Logger    logger{"Graphics Logger"};
    bool                rayTracing{false};
    std::string         applicationName{"Illumination Engine"};
    IEVersion           applicationVersion{0, 0, 1};
    IEVersion           minimumVulkanVersion{1, 0, 0};
    IEVersion           desiredVulkanVersion{1, 2, 0};
    uint8_t             msaaSamples{VK_SAMPLE_COUNT_1_BIT};
    float               anisotropicFilterLevel{16.0f};
    bool                mipMapping{true};
    float               mipMapLevel{0};
    bool                fullscreen{false};
    int                 refreshRate{60};
    bool                vSync{true};
    GLFWmonitor        *primaryMonitor;
    std::array<int, 2>  defaultResolution{};
    std::array<int, 2>  fullscreenResolution{};
    std::array<int, 2>  windowedResolution{};
    std::array<int, 2> *currentResolution;
    std::array<int, 2>  defaultPosition{};
    std::array<int, 2>  fullscreenPosition{};
    std::array<int, 2>  windowedPosition{};
    std::array<int, 2> *currentPosition;
    double              fov{90};
    double              renderDistance{1000000};
    double              mouseSensitivity{0.1};
    float               movementSpeed{2.5};
};
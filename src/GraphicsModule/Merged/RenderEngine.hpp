#pragma once

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#endif

#include <log4cplus/logger.h>
#include <log4cplus/loglevel.h>
#include <log4cplus/layout.h>
#include <log4cplus/fileappender.h>
#include <log4cplus/consoleappender.h>

#include "RenderEngineLink.hpp"

class RenderEngine {
public:
    RenderEngineLink renderEngineLink;

    explicit RenderEngine(const std::string& API = "OpenGL") {
        log4cplus::SharedAppenderPtr consoleAppender{new log4cplus::ConsoleAppender()};
        consoleAppender->setName("consoleLogger");
        log4cplus::SharedAppenderPtr fileAppender{new log4cplus::FileAppender("IlluminationEngineGraphicsModule.log")};
        fileAppender->setName("consoleLogger");
        std::unique_ptr<log4cplus::Layout> layout;
        layout = std::unique_ptr<log4cplus::Layout>(new log4cplus::SimpleLayout);
        consoleAppender->setLayout(reinterpret_cast<std::unique_ptr<log4cplus::Layout> &&>(layout));
        layout = std::unique_ptr<log4cplus::Layout>(new log4cplus::TTCCLayout);
        fileAppender->setLayout(reinterpret_cast<std::unique_ptr<log4cplus::Layout> &&>(layout));
        renderEngineLink.graphicsModuleLogger = log4cplus::Logger::getInstance("IlluminationEngineGraphicsModuleLogger");
        renderEngineLink.graphicsModuleLogger.addAppender(consoleAppender);
        renderEngineLink.graphicsModuleLogger.addAppender(fileAppender);
        renderEngineLink.graphicsModuleLogger.log(log4cplus::INFO_LOG_LEVEL, "Creating render engine...");
        renderEngineLink.api.name = API;
#ifdef ILLUMINATION_ENGINE_VULKAN
        if (renderEngineLink.api.name == "Vulkan") {
            renderEngineLink.api.getVersion();
            vkb::detail::Result<vkb::SystemInfo> systemInfo = vkb::SystemInfo::get_system_info();
            vkb::InstanceBuilder builder;
            builder.set_app_name(renderEngineLink.settings.applicationName.c_str()).set_app_version(renderEngineLink.settings.applicationVersion.major, renderEngineLink.settings.applicationVersion.minor, renderEngineLink.settings.applicationVersion.patch).require_api_version(renderEngineLink.api.version.major, renderEngineLink.api.version.minor, renderEngineLink.api.version.patch);
#ifndef NDEBUG
            if (systemInfo->validation_layers_available) { builder.request_validation_layers(); }
            if (systemInfo->debug_utils_available) { builder.use_default_debug_messenger(); }
#endif
            vkb::detail::Result<vkb::Instance> instanceBuilder = builder.build();
            if (!instanceBuilder) { renderEngineLink.graphicsModuleLogger.log(log4cplus::DEBUG_LOG_LEVEL, "Failed to create Vulkan instance. Error: " + instanceBuilder.error().message() + "\n"); }
            renderEngineLink.instance = instanceBuilder.value();
        }
#endif
    }

    void create() {
        if (!glfwInit()) { std::cerr << "GLFW failed to initialize\n\tNo way to recover" << std::endl; }
#ifdef ILLUMINATION_ENGINE_VULKAN
        if (renderEngineLink.api.name == "Vulkan") { glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); }
#endif
#ifdef ILLUMINATION_ENGINE_OPENGL
        if (renderEngineLink.api.name == "OpenGL") {
#ifdef __APPLE__
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
#ifndef NDEBUG
            glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
        }
#endif
#endif
        renderEngineLink.window = glfwCreateWindow(800, 600, "ApplicationNameHere", nullptr, nullptr);
        glfwMakeContextCurrent(renderEngineLink.window);
        glewInit();
#ifdef ILLUMINATION_ENGINE_OPENGL
        if (renderEngineLink.api.name == "OpenGL") { renderEngineLink.api.getVersion(); }
#endif
        printf("%s %s\n", renderEngineLink.api.name.c_str(), renderEngineLink.api.version.name.c_str());
        int width, height, channels, sizes[] = {256, 128, 64, 32, 16};
        GLFWimage icons[sizeof(sizes) / sizeof(int)];
        for (unsigned long i = 0; i < sizeof(sizes) / sizeof(int); ++i) {
            std::string filename = "res/Logos/IlluminationEngineLogo" + std::to_string(sizes[i]) + ".png";
            stbi_uc *pixels = stbi_load(filename.c_str(), &width, &height, &channels, STBI_rgb_alpha);
            if (!pixels) { throw std::runtime_error("failed to prepare texture image from file: " + filename); }
            icons[i].pixels = pixels;
            icons[i].height = height;
            icons[i].width = width;
        }
        glfwSetWindowIcon(renderEngineLink.window, sizeof(icons) / sizeof(GLFWimage), icons);
        for (GLFWimage icon: icons) { stbi_image_free(icon.pixels); }
        glfwSetWindowSizeLimits(renderEngineLink.window, 1, 1, GLFW_DONT_CARE, GLFW_DONT_CARE);
    }

    ~RenderEngine() {
        glfwTerminate();
    }
};
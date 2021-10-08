#pragma once

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#endif

#include "IeBuffer.hpp"

#ifndef ILLUMINATION_ENGINE_VULKAN
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkImage)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkFormat)
#endif

enum IePreDesignedImage {
    DEPTH = 0x00000000,
    COLOR = 0x00000001,
    TEXTURE = 0x00000002
};

enum IeImageType {
    IE_IMAGE_TYPE_ARRAY_BIT = 0b1000,
    IE_IMAGE_TYPE_MULTISAMPLE_BIT = 0b0100,
    IE_IMAGE_TYPE_DIMENSIONS_BIT = 0b0011,
    IE_IMAGE_TYPE_1D = 0b0000,
    IE_IMAGE_TYPE_1D_ARRAY = 0b0100,
    IE_IMAGE_TYPE_2D = 0b0001,
    IE_IMAGE_TYPE_2D_ARRAY = 0b0101,
    IE_IMAGE_TYPE_2D_MULTISAMPLE = 0b1001,
    IE_IMAGE_TYPE_2D_MULTISAMPLE_ARRAY = 0b1101,
    IE_IMAGE_TYPE_3D = 0b0010,
    IE_IMAGE_TYPE_CUBE = 0b0011,
    IE_IMAGE_TYPE_CUBE_ARRAY = 0b0111
};

enum IeImageFilter {
    IE_IMAGE_FILTER_MIPMAP_METHOD_BIT = 0b100,
    IE_IMAGE_FILTER_MIPMAP_ENABLED_BIT = 0b010,
    IE_IMAGE_FILTER_METHOD_BIT = 0b001,
    IE_IMAGE_FILTER_NONE = 0b000,
    IE_IMAGE_FILTER_NEAREST = 0b000,
    IE_IMAGE_FILTER_LINEAR = 0b001,
    IE_IMAGE_FILTER_NEAREST_MIPMAP_NEAREST = 0b010,
    IE_IMAGE_FILTER_NEAREST_MIPMAP_LINEAR = 0b110,
    IE_IMAGE_FILTER_LINEAR_MIPMAP_NEAREST = 0b011,
    IE_IMAGE_FILTER_LINEAR_MIPMAP_LINEAR = 0b111
};

#ifdef ILLUMINATION_ENGINE_VULKAN
const static std::unordered_multimap<unsigned char, std::pair<VkImageType, int>> ieImageTypes {
        {IE_IMAGE_TYPE_1D,                      {VK_IMAGE_TYPE_1D,      GL_IMAGE_1D}},
        {IE_IMAGE_TYPE_1D_ARRAY,                {VK_IMAGE_TYPE_1D,      GL_IMAGE_1D_ARRAY}},
        {IE_IMAGE_TYPE_2D,                      {VK_IMAGE_TYPE_2D,      GL_IMAGE_2D}},
        {IE_IMAGE_TYPE_2D_ARRAY,                {VK_IMAGE_TYPE_2D,      GL_IMAGE_2D_ARRAY}},
        {IE_IMAGE_TYPE_2D_MULTISAMPLE,          {VK_IMAGE_TYPE_2D,      GL_IMAGE_2D_MULTISAMPLE}},
        {IE_IMAGE_TYPE_2D_MULTISAMPLE_ARRAY,    {VK_IMAGE_TYPE_2D,      GL_IMAGE_2D_MULTISAMPLE_ARRAY}},
        {IE_IMAGE_TYPE_3D,                      {VK_IMAGE_TYPE_3D,      GL_IMAGE_3D}},
        {IE_IMAGE_TYPE_CUBE,                    {VK_IMAGE_TYPE_2D,      GL_IMAGE_CUBE}},
        {IE_IMAGE_TYPE_CUBE_ARRAY,              {VK_IMAGE_TYPE_2D,      GL_IMAGE_CUBE_MAP_ARRAY}},
};

const static std::unordered_multimap<unsigned char, std::pair<VkFilter, int>> ieImageFilters{
        {IE_IMAGE_FILTER_NONE,                   {VK_FILTER_NEAREST,    GL_NEAREST}},
        {IE_IMAGE_FILTER_NEAREST,                {VK_FILTER_NEAREST,    GL_NEAREST}},
        {IE_IMAGE_FILTER_LINEAR,                 {VK_FILTER_LINEAR,     GL_LINEAR}},
        {IE_IMAGE_FILTER_NEAREST_MIPMAP_NEAREST, {VK_FILTER_NEAREST,    GL_NEAREST_MIPMAP_NEAREST}},
        {IE_IMAGE_FILTER_NEAREST_MIPMAP_LINEAR,  {VK_FILTER_LINEAR,     GL_NEAREST_MIPMAP_LINEAR}},
        {IE_IMAGE_FILTER_LINEAR_MIPMAP_NEAREST,  {VK_FILTER_LINEAR,     GL_LINEAR_MIPMAP_NEAREST}},
        {IE_IMAGE_FILTER_LINEAR_MIPMAP_LINEAR,   {VK_FILTER_LINEAR,     GL_LINEAR_MIPMAP_LINEAR}},
};
#else
#ifdef ILLUMINATION_ENGINE_OPENGL
const static std::unordered_multimap<unsigned long long, std::pair<int, int>> ieImageTypes {
        {IE_IMAGE_TYPE_1D,                          {GL_IMAGE_1D,                       GL_IMAGE_1D}},
        {IE_IMAGE_TYPE_1D_ARRAY,                    {GL_IMAGE_1D_ARRAY,                 GL_IMAGE_1D_ARRAY}},
        {IE_IMAGE_TYPE_2D,                          {GL_IMAGE_2D,                       GL_IMAGE_2D}},
        {IE_IMAGE_TYPE_2D_ARRAY,                    {GL_IMAGE_2D_ARRAY,                 GL_IMAGE_2D_ARRAY}},
        {IE_IMAGE_TYPE_2D_MULTISAMPLE,              {GL_IMAGE_2D_MULTISAMPLE,           GL_IMAGE_2D_MULTISAMPLE}},
        {IE_IMAGE_TYPE_2D_MULTISAMPLE_ARRAY,        {GL_IMAGE_2D_MULTISAMPLE_ARRAY,     GL_IMAGE_2D_MULTISAMPLE_ARRAY}},
        {IE_IMAGE_TYPE_3D,                          {GL_IMAGE_3D,                       GL_IMAGE_3D}},
        {IE_IMAGE_TYPE_CUBE,                        {GL_IMAGE_CUBE,                     GL_IMAGE_CUBE}},
        {IE_IMAGE_TYPE_CUBE_ARRAY,                  {GL_IMAGE_CUBE_MAP_ARRAY,           GL_IMAGE_CUBE_MAP_ARRAY}},
};

const static std::unordered_multimap<unsigned char, std::pair<int, int>> ieImageFilters{
        {IE_IMAGE_FILTER_NONE,                      {GL_NEAREST,                        GL_NEAREST}},
        {IE_IMAGE_FILTER_NEAREST,                   {GL_NEAREST,                        GL_NEAREST}},
        {IE_IMAGE_FILTER_LINEAR,                    {GL_LINEAR,                         GL_LINEAR}},
        {IE_IMAGE_FILTER_NEAREST_MIPMAP_NEAREST,    {GL_NEAREST_MIPMAP_NEAREST,         GL_NEAREST_MIPMAP_NEAREST}},
        {IE_IMAGE_FILTER_NEAREST_MIPMAP_LINEAR,     {GL_NEAREST_MIPMAP_LINEAR,          GL_NEAREST_MIPMAP_LINEAR}},
        {IE_IMAGE_FILTER_LINEAR_MIPMAP_NEAREST,     {GL_LINEAR_MIPMAP_NEAREST,          GL_LINEAR_MIPMAP_NEAREST}},
        {IE_IMAGE_FILTER_LINEAR_MIPMAP_LINEAR,      {GL_LINEAR_MIPMAP_LINEAR,           GL_LINEAR_MIPMAP_LINEAR}},
};
#endif
#endif

class IeImage {
public:
    struct Properties{
    public:
        unsigned int mipLevels{}; // Mip mapping levels
        unsigned int aspect{}; // Vulkan: VkImageAspectFlagsBit,        OpenGL:
        unsigned int format{}; // Vulkan: VkFormat,                     OpenGL:
        enum IeImageFilter filter{};
        enum IeImageType type{};
        unsigned int memoryUsage{}; // Vulkan: VmaMemoryUsage,          OpenGL:
        unsigned int usage{}; // Vulkan: VkImageUsageFlags,             OpenGL:
        unsigned int layout{}; // Vulkan: VkImageLayout,                OpenGL:
        unsigned int msaaSamples{};
        unsigned int tiling{};
        unsigned int width{1};
        unsigned int height{1};
        unsigned int depth{1};
    };

    struct CreateInfo {
    public:
        //Required
        std::variant<IePreDesignedImage, Properties> specifications{};

        //Only required if specifications != TEXTURE
        int width{}, height{};
        uint8_t msaaSamples{1};

        //Only required if specifications == TEXTURE
        std::string filename{};

        //Optional
        /**@todo: Double check that this can be stored as a string and still behave as expected.*/
        std::string *data{};

        //Optional - Only has an effect when using the Vulkan IeAPI.
        IeBuffer *dataSource{};
    };

    struct Created {
        bool image{};
        bool view{};
        bool sampler{};
        bool loaded{};
    };

    /**@todo: Separate the fields that can be made private so that they can be made so.*/
    #ifdef ILLUMINATION_ENGINE_VULKAN
    VkImageView view{};
    VmaAllocation allocation{};
    #endif
    CreateInfo createdWith{};
    IeCommandPool *commandPool{};
    unsigned int commandBufferIndex{};
    IeRenderEngineLink *linkedRenderEngine{};
    std::variant<VkImage, unsigned int> image{};
    Created created{};
    Properties imageProperties{};

    virtual void create(IeRenderEngineLink *engineLink, CreateInfo *createInfo) {
        createdWith = *createInfo;
        linkedRenderEngine = engineLink;
        imageProperties.mipLevels = std::min(std::max(static_cast<unsigned int>(std::floor(std::log2(std::max(createdWith.width, createdWith.height)) + 1) * (imageProperties.filter & IE_IMAGE_FILTER_MIPMAP_ENABLED_BIT)), static_cast<unsigned int>(1)), linkedRenderEngine->settings.maxMipLevels);
        createdWith.msaaSamples = std::min(createdWith.msaaSamples, linkedRenderEngine->settings.msaaSamples);
        if (createdWith.specifications.index() == 0) {
            imageProperties.aspect = std::get<IePreDesignedImage>(createdWith.specifications) == DEPTH ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
            imageProperties.type = std::get<IePreDesignedImage>(createdWith.specifications) == TEXTURE ? IE_IMAGE_TYPE_2D : createdWith.msaaSamples > 1 ? IE_IMAGE_TYPE_2D_MULTISAMPLE : IE_IMAGE_TYPE_2D;
            if (std::get<IePreDesignedImage>(createdWith.specifications) == TEXTURE) {
                if (!createdWith.width) { imageProperties.width = createdWith.width; }
                if (!createdWith.height) { imageProperties.height = createdWith.height; }
            } else {
                imageProperties.width = createdWith.width ? createdWith.width : linkedRenderEngine->swapchain.extent.width;
                imageProperties.height = createdWith.height ? createdWith.height : linkedRenderEngine->swapchain.extent.height;
            }
        } else if (createdWith.specifications.index() == 1) {
            imageProperties.aspect = VK_IMAGE_ASPECT_COLOR_BIT;
            imageProperties.type = std::get<Properties>(createdWith.specifications).type;
            imageProperties.format = std::get<Properties>(createdWith.specifications).format;
        }
        #ifdef ILLUMINATION_ENGINE_OPENGL
        if (linkedRenderEngine->api.name == "OpenGL") {
            glGenTextures(1, &std::get<unsigned int>(image));
            imageProperties.format = createdWith.msaaSamples > 1 ? ieImageTypes.find(IE_IMAGE_TYPE_2D_MULTISAMPLE)->second.second : ieImageTypes.find(IE_IMAGE_TYPE_2D)->second.second;
            created.image = true;
        }
        #endif
    }

    virtual void transitionLayout(unsigned int newLayout, VkCommandBuffer commandBuffer) {

    }

    virtual void unload() {
        if (created.loaded) { stbi_image_free(createdWith.data); }
        created.loaded = false;
        #ifdef ILLUMINATION_ENGINE_VULKAN
        if (linkedRenderEngine->api.name == "Vulkan") {
            linkedRenderEngine->log->log("Freed image bufferData for image at " + std::to_string(reinterpret_cast<unsigned long long int>(&image)), log4cplus::INFO_LOG_LEVEL, "Graphics Module");
            if (created.view) { vkDestroyImageView(linkedRenderEngine->device.device, view, nullptr); }
            if (created.image) { vmaDestroyImage(linkedRenderEngine->allocator, std::get<VkImage>(image), allocation); }
        }
        #endif
        #ifdef ILLUMINATION_ENGINE_OPENGL
        if (linkedRenderEngine->api.name == "OpenGL") { linkedRenderEngine->log->log("Freed image bufferData for image " + std::to_string(std::get<unsigned int>(image)), log4cplus::INFO_LOG_LEVEL, "Graphics Module"); }
        #endif
    }

    virtual void upload() {
        #ifdef ILLUMINATION_ENGINE_VULKAN
        if (linkedRenderEngine->api.name == "Vulkan") {
            VkImageSubresourceRange subresourceRange{.aspectMask=imageProperties.aspect, .baseMipLevel=0, .levelCount=1, .baseArrayLayer=0, .layerCount=1};
            VkImageViewCreateInfo imageViewCreateInfo{.sType=VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, .viewType=VK_IMAGE_VIEW_TYPE_2D, .format=static_cast<VkFormat>(imageProperties.format), .subresourceRange=subresourceRange};
            VmaAllocationCreateInfo allocationCreateInfo{.usage=static_cast<VmaMemoryUsage>(imageProperties.memoryUsage)};
            VkExtent3D extent{.width=imageProperties.width, .height=imageProperties.height, .depth=imageProperties.depth};
            VkImageCreateInfo imageCreateInfo{.sType=VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, .imageType=static_cast<VkImageType>(imageProperties.type), .format=static_cast<VkFormat>(imageProperties.format), .extent=extent, .mipLevels=imageProperties.mipLevels, .arrayLayers=1, .samples=static_cast<VkSampleCountFlagBits>(imageProperties.msaaSamples), .tiling=static_cast<VkImageTiling>(imageProperties.tiling), .usage=imageProperties.usage, .sharingMode=VK_SHARING_MODE_EXCLUSIVE, .initialLayout=VK_IMAGE_LAYOUT_UNDEFINED};
            if (vmaCreateImage(linkedRenderEngine->allocator, &imageCreateInfo, &allocationCreateInfo, &std::get<VkImage>(image), &allocation, nullptr) != VK_SUCCESS) { linkedRenderEngine->log->log("Failed to create image!", log4cplus::WARN_LOG_LEVEL, "Graphics Module"); }
            created.image = true;
            imageViewCreateInfo.image = std::get<VkImage>(image);
            if (vkCreateImageView(linkedRenderEngine->device.device, &imageViewCreateInfo, nullptr, &view) != VK_SUCCESS) { linkedRenderEngine->log->log("Failed to create image view!", log4cplus::WARN_LOG_LEVEL, "Graphics Module"); }
            created.view = true;
            auto imageLayout = static_cast<VkImageLayout>(imageProperties.layout);
            if (createdWith.dataSource != nullptr) {
                transitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, commandPool->commandBuffers[commandBufferIndex]);
                createdWith.dataSource->toImage(*this, imageProperties.width, imageProperties.height, commandPool->commandBuffers[commandBufferIndex]);
            }
            if (imageProperties.layout != imageLayout) { transitionLayout(imageProperties.layout, commandPool->commandBuffers[commandBufferIndex]); }
        }
        #endif
        #ifdef ILLUMINATION_ENGINE_OPENGL
        if (linkedRenderEngine->api.name == "OpenGL") {
            glBindTexture(imageProperties.type, std::get<unsigned int>(image));
            glTexParameteri(imageProperties.type, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(imageProperties.type, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(imageProperties.type, GL_TEXTURE_MIN_FILTER, ieImageFilters.find(imageProperties.filter)->second.second);
            glTexParameteri(imageProperties.type, GL_TEXTURE_MAG_FILTER, ieImageFilters.find(imageProperties.filter)->second.second);
            glTexImage2D(imageProperties.type, 0, GL_RGB, createdWith.width, createdWith.height, 0, GL_RGB, GL_UNSIGNED_BYTE, createdWith.data);
            glTexImage2D(imageProperties.type, 0, GL_DEPTH24_STENCIL8, createdWith.width, createdWith.height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, createdWith.data);
            if (imageProperties.filter & IE_IMAGE_FILTER_MIPMAP_ENABLED_BIT) { glGenerateMipmap(imageProperties.type); }
        }
        #endif
        created.loaded = true;
    }

    virtual void toBuffer(const IeBuffer &buffer, VkCommandBuffer commandBuffer) {}

    virtual void destroy() {
        unload();
        #ifdef ILLUMINATION_ENGINE_VULKAN
        if (created.image) { vmaDestroyImage(linkedRenderEngine->allocator, std::get<VkImage>(image), allocation); }
        #endif
        #ifdef ILLUMINATION_ENGINE_OPENGL
        if (created.image) { glDeleteTextures(1, &std::get<unsigned int>(image)); }
        #endif
        created.image = false;
    }
};

void IeBuffer::toImage(IeImage &image, unsigned int width, unsigned int height, VkCommandBuffer commandBuffer) {
    if (!created) { linkedRenderEngine->log->log("Called IeBuffer::toImage() on a IeBuffer that does not exist!", log4cplus::WARN_LOG_LEVEL, "Graphics Module"); }
    /**@todo: Properly handle an uncreated IeBuffer. i.e. Make one.*/
    /**@todo: Finish making this function.*/
}
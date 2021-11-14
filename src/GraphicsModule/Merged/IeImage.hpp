#pragma once

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#endif

#include "IeBuffer.hpp"

#include <error.h>

enum IePreDesignedImage {
    IE_PRE_DESIGNED_DEPTH_IMAGE = 0x00000000,
    IE_PRE_DESIGNED_COLOR_IMAGE = 0x00000001,
    IE_PRE_DESIGNED_TEXTURE_IMAGE = 0x00000002
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

enum IeImageFormat {
    IE_IMAGE_FORMAT_COLORSPACE_BIT = 0b1110000000,
    IE_IMAGE_FORMAT_CHANNELS_BIT = 0b0001100000,
    IE_IMAGE_FORMAT_BIT_DEPTH_BIT = 0b0000011111,
    IE_IMAGE_FORMAT_SRGB_RGB_8BIT = 0b0001001000,
    IE_IMAGE_FORMAT_SRGB_RGBA_8BIT = 0b0001101000,
    IE_IMAGE_FORMAT_UINT_SFLOAT_32BIT = 0b0010010000
};

enum IeImageUsage {
    IE_IMAGE_USAGE_COLOR_READ = 0b0000,
    IE_IMAGE_USAGE_COLOR_WRITE = 0b0001,
    IE_IMAGE_USAGE_DEPTH_READ = 0b0010,
    IE_IMAGE_USAGE_DEPTH_WRITE = 0b0011,
    IE_IMAGE_USAGE_READ = 0b0100,
    IE_IMAGE_USAGE_WRITE = 0b0101,
    IE_IMAGE_USAGE_DISPLAY = 0b0110,
    IE_IMAGE_USAGE_DESTINATION = 0b0111,
    IE_IMAGE_USAGE_SOURCE = 0b1000,
    IE_IMAGE_USAGE_UNDEFINED = 0b1001,
    IE_IMAGE_USAGE_ANY = 0b1010
};

enum IeImageTiling {
    IE_IMAGE_TILING_REPEAT_BIT = 0b001,
    IE_IMAGE_TILING_MIRROR_BIT = 0b010,
    IE_IMAGE_TILING_CLAMP_BIT = 0b100,
    IE_IMAGE_TILING_REPEAT = 0b001,
    IE_IMAGE_TILING_MIRROR_REPEAT = 0b011,
    IE_IMAGE_TILING_CLAMP = 0b100,
    IE_IMAGE_TILING_MIRROR_CLAMP = 0b110
};

#ifdef ILLUMINATION_ENGINE_VULKAN
const static std::unordered_multimap<IeImageType, std::pair<VkImageType, uint32_t>> ieImageTypes {
        {IE_IMAGE_TYPE_1D,                          {VK_IMAGE_TYPE_1D,              GL_IMAGE_1D}},
        {IE_IMAGE_TYPE_1D_ARRAY,                    {VK_IMAGE_TYPE_1D,              GL_IMAGE_1D_ARRAY}},
        {IE_IMAGE_TYPE_2D,                          {VK_IMAGE_TYPE_2D,              GL_IMAGE_2D}},
        {IE_IMAGE_TYPE_2D_ARRAY,                    {VK_IMAGE_TYPE_2D,              GL_IMAGE_2D_ARRAY}},
        {IE_IMAGE_TYPE_2D_MULTISAMPLE,              {VK_IMAGE_TYPE_2D,              GL_IMAGE_2D_MULTISAMPLE}},
        {IE_IMAGE_TYPE_2D_MULTISAMPLE_ARRAY,        {VK_IMAGE_TYPE_2D,              GL_IMAGE_2D_MULTISAMPLE_ARRAY}},
        {IE_IMAGE_TYPE_3D,                          {VK_IMAGE_TYPE_3D,              GL_IMAGE_3D}},
        {IE_IMAGE_TYPE_CUBE,                        {VK_IMAGE_TYPE_2D,              GL_IMAGE_CUBE}},
        {IE_IMAGE_TYPE_CUBE_ARRAY,                  {VK_IMAGE_TYPE_2D,              GL_IMAGE_CUBE_MAP_ARRAY}}
};

const static std::unordered_multimap<IeImageFilter, std::pair<VkFilter, uint32_t>> ieImageFilters {
        {IE_IMAGE_FILTER_NONE,                      {VK_FILTER_NEAREST,             GL_NEAREST}},
        {IE_IMAGE_FILTER_NEAREST,                   {VK_FILTER_NEAREST,             GL_NEAREST}},
        {IE_IMAGE_FILTER_LINEAR,                    {VK_FILTER_LINEAR,              GL_LINEAR}},
        {IE_IMAGE_FILTER_NEAREST_MIPMAP_NEAREST,    {VK_FILTER_NEAREST,             GL_NEAREST_MIPMAP_NEAREST}},
        {IE_IMAGE_FILTER_NEAREST_MIPMAP_LINEAR,     {VK_FILTER_LINEAR,              GL_NEAREST_MIPMAP_LINEAR}},
        {IE_IMAGE_FILTER_LINEAR_MIPMAP_NEAREST,     {VK_FILTER_LINEAR,              GL_LINEAR_MIPMAP_NEAREST}},
        {IE_IMAGE_FILTER_LINEAR_MIPMAP_LINEAR,      {VK_FILTER_LINEAR,              GL_LINEAR_MIPMAP_LINEAR}}
};

const static std::unordered_multimap<IeImageFormat, std::pair<VkFormat, uint32_t>> ieImageFormats{
        {IE_IMAGE_FORMAT_SRGB_RGB_8BIT,             {VK_FORMAT_R8G8B8_SRGB,         GL_RGB8}},
        {IE_IMAGE_FORMAT_SRGB_RGBA_8BIT,            {VK_FORMAT_R8G8B8A8_SRGB,       GL_RGBA8}},
        {IE_IMAGE_FORMAT_UINT_SFLOAT_32BIT,         {VK_FORMAT_D32_SFLOAT_S8_UINT,  GL_FLOAT_32_UNSIGNED_INT_24_8_REV}}
};

const static std::unordered_multimap<IeImageUsage, std::pair<VkImageLayout, uint32_t>> ieImageUsages {
        {IE_IMAGE_USAGE_COLOR_READ,         {VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,              GL_NONE}},
        {IE_IMAGE_USAGE_COLOR_WRITE,        {VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,              GL_NONE}},
        {IE_IMAGE_USAGE_DEPTH_READ,         {VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL_KHR,           GL_NONE}},
        {IE_IMAGE_USAGE_DEPTH_WRITE,        {VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR,          GL_NONE}},
        {IE_IMAGE_USAGE_READ,               {VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR,                 GL_NONE}},
        {IE_IMAGE_USAGE_WRITE,              {VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR,                GL_NONE}},
        {IE_IMAGE_USAGE_DISPLAY,            {VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,                       GL_NONE}},
        {IE_IMAGE_USAGE_DESTINATION,        {VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,                  GL_NONE}},
        {IE_IMAGE_USAGE_SOURCE,             {VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,                  GL_NONE}},
        {IE_IMAGE_USAGE_UNDEFINED,          {VK_IMAGE_LAYOUT_UNDEFINED,                             GL_NONE}},
        {IE_IMAGE_USAGE_ANY,                {VK_IMAGE_LAYOUT_GENERAL,                               GL_NONE}}
};

const static std::unordered_multimap<IeImageTiling, std::pair<VkSamplerAddressMode, uint32_t>> ieImageTilings{
        {IE_IMAGE_TILING_REPEAT,            {VK_SAMPLER_ADDRESS_MODE_REPEAT,                GL_REPEAT}},
        {IE_IMAGE_TILING_MIRROR_REPEAT,     {VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,       GL_MIRRORED_REPEAT}},
        {IE_IMAGE_TILING_CLAMP,             {VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,         GL_CLAMP_TO_EDGE}},
        {IE_IMAGE_TILING_MIRROR_CLAMP,      {VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE,  GL_MIRROR_CLAMP_TO_EDGE}}
};
#else
#ifdef ILLUMINATION_ENGINE_OPENGL
const static std::unordered_multimap<IeImageType, std::pair<uint32_t, uint32_t>> ieImageTypes {
        {IE_IMAGE_TYPE_1D,                                  {GL_IMAGE_1D,                       GL_IMAGE_1D}},
        {IE_IMAGE_TYPE_1D_ARRAY,                            {GL_IMAGE_1D_ARRAY,                 GL_IMAGE_1D_ARRAY}},
        {IE_IMAGE_TYPE_2D,                                  {GL_IMAGE_2D,                       GL_IMAGE_2D}},
        {IE_IMAGE_TYPE_2D_ARRAY,                            {GL_IMAGE_2D_ARRAY,                 GL_IMAGE_2D_ARRAY}},
        {IE_IMAGE_TYPE_2D_MULTISAMPLE,                      {GL_IMAGE_2D_MULTISAMPLE,           GL_IMAGE_2D_MULTISAMPLE}},
        {IE_IMAGE_TYPE_2D_MULTISAMPLE_ARRAY,                {GL_IMAGE_2D_MULTISAMPLE_ARRAY,     GL_IMAGE_2D_MULTISAMPLE_ARRAY}},
        {IE_IMAGE_TYPE_3D,                                  {GL_IMAGE_3D,                       GL_IMAGE_3D}},
        {IE_IMAGE_TYPE_CUBE,                                {GL_IMAGE_CUBE,                     GL_IMAGE_CUBE}},
        {IE_IMAGE_TYPE_CUBE_ARRAY,                          {GL_IMAGE_CUBE_MAP_ARRAY,           GL_IMAGE_CUBE_MAP_ARRAY}},
};

const static std::unordered_multimap<IeImageFilter, std::pair<uint32_t, uint32_t>> ieImageFilters{
        {IE_IMAGE_FILTER_NONE,                              {GL_NEAREST,                        GL_NEAREST}},
        {IE_IMAGE_FILTER_NEAREST,                           {GL_NEAREST,                        GL_NEAREST}},
        {IE_IMAGE_FILTER_LINEAR,                            {GL_LINEAR,                         GL_LINEAR}},
        {IE_IMAGE_FILTER_NEAREST_MIPMAP_NEAREST,            {GL_NEAREST_MIPMAP_NEAREST,         GL_NEAREST_MIPMAP_NEAREST}},
        {IE_IMAGE_FILTER_NEAREST_MIPMAP_LINEAR,             {GL_NEAREST_MIPMAP_LINEAR,          GL_NEAREST_MIPMAP_LINEAR}},
        {IE_IMAGE_FILTER_LINEAR_MIPMAP_NEAREST,             {GL_LINEAR_MIPMAP_NEAREST,          GL_LINEAR_MIPMAP_NEAREST}},
        {IE_IMAGE_FILTER_LINEAR_MIPMAP_LINEAR,              {GL_LINEAR_MIPMAP_LINEAR,           GL_LINEAR_MIPMAP_LINEAR}},
};

const static std::unordered_multimap<IeImageFormat, std::pair<uint32_t, uint32_t>> ieImageFormats{
        {IE_IMAGE_FORMAT_R8G8B8_SRGB,                       {GL_RGB8,                           GL_RGB8}},
        {IE_IMAGE_FORMAT_SRGB_RGBA_8BIT,                    {GL_RGBA8,                          GL_RGBA8}},
        {IE_IMAGE_FORMAT_UINT_SFLOAT_32BIT,                 {GL_FLOAT_32_UNSIGNED_INT_24_8_REV, GL_FLOAT_32_UNSIGNED_INT_24_8_REV}}
};

const static std::unordered_multimap<IeImageUsage, std::pair<uint32_t , uint32_t>> ieImageUsages {
        {IE_IMAGE_USAGE_COLOR_READ,                         {GL_NONE,                           GL_NONE}},
        {IE_IMAGE_USAGE_COLOR_WRITE,                        {GL_NONE,                           GL_NONE}},
        {IE_IMAGE_USAGE_DEPTH_READ,                         {GL_NONE,                           GL_NONE}},
        {IE_IMAGE_USAGE_DEPTH_WRITE,                        {GL_NONE,                           GL_NONE}},
        {IE_IMAGE_USAGE_READ,                               {GL_NONE,                           GL_NONE}},
        {IE_IMAGE_USAGE_WRITE,                              {GL_NONE,                           GL_NONE}},
        {IE_IMAGE_USAGE_DISPLAY,                            {GL_NONE,                           GL_NONE}},
        {IE_IMAGE_USAGE_DESTINATION,                        {GL_NONE,                           GL_NONE}},
        {IE_IMAGE_USAGE_SOURCE,                             {GL_NONE,                           GL_NONE}},
        {IE_IMAGE_USAGE_UNDEFINED,                          {GL_NONE,                           GL_NONE}},
        {IE_IMAGE_USAGE_ANY,                                {GL_NONE,                           GL_NONE}}
};
#endif
#endif

class IeImage {
public:
    struct Properties {
    public:
        IeImageType type{};
        IeImageFilter filter{};
        IeImageFormat format{};
        IeImageUsage usage{};
        IeImageTiling tiling{};
        uint32_t memoryUsage{};
        uint8_t mipLevels{1};
        uint8_t msaaSamples{1};
        uint16_t width{1};
        uint16_t height{1};
        uint16_t depth{1};
    };

    /**@todo Add a way to override the abstraction layers for more low-level handling of image creation.
     *      Do this by adding another type to the properties variant. One for OpenGL and one for Vulkan that will handle all of the
     *      API-specific stuff for their respective APIs.*/
    struct CreateInfo {
    public:
        //Required
        std::variant<IePreDesignedImage, Properties> properties{};

        //Only required if properties != IE_PRE_DESIGNED_TEXTURE_IMAGE
        uint32_t width{}, height{};
        uint8_t msaaSamples{1};

        //Only required if properties == IE_PRE_DESIGNED_TEXTURE_IMAGE
        std::string filename{};

        //Optional
        std::string data{};

        //Optional - Only has an effect when using the Vulkan API.
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
    uint32_t commandBufferIndex{};
    IeRenderEngineLink *linkedRenderEngine{};
    std::variant<VkImage, uint32_t> image{};
    Created created{};
    Properties imageProperties{};
    bool preDesignedImage;


    virtual void create(IeRenderEngineLink *engineLink, CreateInfo *createInfo) {
        createdWith = *createInfo;
        linkedRenderEngine = engineLink;
        imageProperties.mipLevels = std::min(std::max(static_cast<uint32_t>(std::floor(std::log2(std::max(createdWith.width, createdWith.height)) + 1) * (imageProperties.filter & IE_IMAGE_FILTER_MIPMAP_ENABLED_BIT)), static_cast<uint32_t>(1)), linkedRenderEngine->settings.maxMipLevels);
        createdWith.msaaSamples = std::min(createdWith.msaaSamples, linkedRenderEngine->settings.msaaSamples);
        if (createdWith.properties.index() == 0) {
            preDesignedImage = true;
            if (std::get<IePreDesignedImage>(createdWith.properties) == IE_PRE_DESIGNED_COLOR_IMAGE) {
                imageProperties.type = createdWith.msaaSamples > 1 ? IE_IMAGE_TYPE_2D_MULTISAMPLE : IE_IMAGE_TYPE_2D;
                imageProperties.width = createdWith.width ? createdWith.width : linkedRenderEngine->swapchain.extent.width;
                imageProperties.height = createdWith.height ? createdWith.height : linkedRenderEngine->swapchain.extent.height;
                imageProperties.usage = IE_IMAGE_USAGE_COLOR_WRITE;
                imageProperties.format = IE_IMAGE_FORMAT_SRGB_RGBA_8BIT;
                imageProperties.msaaSamples = createdWith.msaaSamples;
                imageProperties.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;
            } if (std::get<IePreDesignedImage>(createdWith.properties) == IE_PRE_DESIGNED_DEPTH_IMAGE) {
                imageProperties.type = createdWith.msaaSamples > 1 ? IE_IMAGE_TYPE_2D_MULTISAMPLE : IE_IMAGE_TYPE_2D;
                imageProperties.width = createdWith.width ? createdWith.width : linkedRenderEngine->swapchain.extent.width;
                imageProperties.height = createdWith.height ? createdWith.height : linkedRenderEngine->swapchain.extent.height;
                imageProperties.usage = IE_IMAGE_USAGE_DEPTH_WRITE;
                imageProperties.format = IE_IMAGE_FORMAT_UINT_SFLOAT_32BIT;
                imageProperties.msaaSamples = createdWith.msaaSamples;
                imageProperties.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;
            } if (std::get<IePreDesignedImage>(createdWith.properties) == IE_PRE_DESIGNED_TEXTURE_IMAGE) {
                imageProperties.type = IE_IMAGE_TYPE_2D;
                imageProperties.usage = IE_IMAGE_USAGE_COLOR_READ;
                imageProperties.format = IE_IMAGE_FORMAT_SRGB_RGBA_8BIT;
                imageProperties.memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
                imageProperties.msaaSamples = 0;
                imageProperties.tiling = IE_IMAGE_TILING_REPEAT;
            }
        } else if (createdWith.properties.index() == 1) {
            preDesignedImage = false;
            imageProperties = std::get<Properties>(createdWith.properties);
        }
        #ifdef ILLUMINATION_ENGINE_OPENGL
        if (linkedRenderEngine->api.name == "OpenGL") {
            glGenTextures(1, &std::get<uint32_t>(image));
            imageProperties.type = createdWith.msaaSamples > 1 ? IE_IMAGE_TYPE_2D_MULTISAMPLE : IE_IMAGE_TYPE_2D;
            created.image = true;
        }
        #endif
    }

    virtual void transitionLayout(uint32_t newLayout, VkCommandBuffer commandBuffer) {

    }

    virtual void unload() {
        if (created.loaded) { stbi_image_free((void *) createdWith.data.c_str()); }
        created.loaded = false;
        #ifdef ILLUMINATION_ENGINE_VULKAN
        if (linkedRenderEngine->api.name == "Vulkan") {
            linkedRenderEngine->log->log("Freed image bufferData for image at " + std::to_string(reinterpret_cast<uint64_t>(&std::get<VkImage>(image))), log4cplus::INFO_LOG_LEVEL, "Graphics Module");
            if (created.view) { vkDestroyImageView(linkedRenderEngine->device.device, view, nullptr); }
            if (created.image) { vmaDestroyImage(linkedRenderEngine->allocator, std::get<VkImage>(image), allocation); }
        }
        #endif
        #ifdef ILLUMINATION_ENGINE_OPENGL
        if (linkedRenderEngine->api.name == "OpenGL") { linkedRenderEngine->log->log("Freed image bufferData for image " + std::to_string(std::get<uint32_t>(image)), log4cplus::INFO_LOG_LEVEL, "Graphics Module"); }
        #endif
    }

    virtual void upload() {
        #ifdef ILLUMINATION_ENGINE_VULKAN
        if (linkedRenderEngine->api.name == "Vulkan") {
            VkImageSubresourceRange subresourceRange{.aspectMask=preDesignedImage ? std::get<IePreDesignedImage>(createdWith.properties) == IE_PRE_DESIGNED_DEPTH_IMAGE ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT : VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel=0, .levelCount=1, .baseArrayLayer=0, .layerCount=1};
            VkImageViewCreateInfo imageViewCreateInfo{.sType=VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, .viewType=VK_IMAGE_VIEW_TYPE_2D, .format=static_cast<VkFormat>(imageProperties.format), .subresourceRange=subresourceRange};
            VmaAllocationCreateInfo allocationCreateInfo{.usage=static_cast<VmaMemoryUsage>(imageProperties.memoryUsage)};
            VkExtent3D extent{.width=imageProperties.width, .height=imageProperties.height, .depth=imageProperties.depth};
            VkImageCreateInfo imageCreateInfo{.sType=VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, .imageType=static_cast<VkImageType>(imageProperties.type), .format=static_cast<VkFormat>(imageProperties.format), .extent=extent, .mipLevels=imageProperties.mipLevels, .arrayLayers=1, .samples=static_cast<VkSampleCountFlagBits>(imageProperties.msaaSamples), .tiling=static_cast<VkImageTiling>(imageProperties.tiling), .usage=imageProperties.usage, .sharingMode=VK_SHARING_MODE_EXCLUSIVE, .initialLayout=VK_IMAGE_LAYOUT_UNDEFINED};
            if (vmaCreateImage(linkedRenderEngine->allocator, &imageCreateInfo, &allocationCreateInfo, &std::get<VkImage>(image), &allocation, nullptr) != VK_SUCCESS) { linkedRenderEngine->log->log("Failed to create image!", log4cplus::WARN_LOG_LEVEL, "Graphics Module"); }
            created.image = true;
            imageViewCreateInfo.image = std::get<VkImage>(image);
            if (vkCreateImageView(linkedRenderEngine->device.device, &imageViewCreateInfo, nullptr, &view) != VK_SUCCESS) { linkedRenderEngine->log->log("Failed to create image view!", log4cplus::WARN_LOG_LEVEL, "Graphics Module"); }
            created.view = true;
            IeImageUsage imageLayout = imageProperties.usage;
            if (createdWith.dataSource != nullptr) {
                transitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, commandPool->commandBuffers[commandBufferIndex]);
                createdWith.dataSource->toImage(this, imageProperties.width, imageProperties.height, commandPool->commandBuffers[commandBufferIndex]);
            }
            if (imageProperties.usage != imageLayout) { transitionLayout(imageProperties.usage, commandPool->commandBuffers[commandBufferIndex]); }
        }
        #endif
        #ifdef ILLUMINATION_ENGINE_OPENGL
        if (linkedRenderEngine->api.name == "OpenGL") {
            glBindTexture(imageProperties.type, std::get<uint32_t>(image));
            glTexParameteri(imageProperties.type, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(imageProperties.type, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(imageProperties.type, GL_TEXTURE_MIN_FILTER, static_cast<int>(ieImageFilters.find(imageProperties.filter)->second.second));
            glTexParameteri(imageProperties.type, GL_TEXTURE_MAG_FILTER, static_cast<int>(ieImageFilters.find(imageProperties.filter)->second.second));
            glTexImage2D(imageProperties.type, 0, GL_RGB, static_cast<int>(createdWith.width), static_cast<int>(createdWith.height), 0, GL_RGB, GL_UNSIGNED_BYTE, createdWith.data.c_str());
            glTexImage2D(imageProperties.type, 0, GL_DEPTH24_STENCIL8, static_cast<int>(createdWith.width), static_cast<int>(createdWith.height), 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, createdWith.data.c_str());
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
        if (created.image) { glDeleteTextures(1, &std::get<uint32_t>(image)); }
        #endif
        created.image = false;
    }
};

IeImage IeBuffer::toImage(IeImage* image, uint32_t width, uint32_t height, VkCommandBuffer commandBuffer) {
    if (!created) {
        linkedRenderEngine->log->log("Called IeBuffer::toImage() on a IeBuffer that does not exist!", log4cplus::ERROR_LOG_LEVEL, "Graphics Module");
    }
    if (image == nullptr) {
        linkedRenderEngine->log->log("Called IeBuffer::toImage() with an IeImage that does not exist!", log4cplus::ERROR_LOG_LEVEL, "Graphics Module");
    }
    if (!image->created.image) {
        linkedRenderEngine->log->log("Called IeBuffer::toImage() with an IeImage that has not been created!", log4cplus::WARN_LOG_LEVEL, "Graphics Module");
    }
    /**@todo Properly handle an uncreated IeImage. i.e. Make one.*/

    /**@todo Finish making this function.*/
    return *image;
}
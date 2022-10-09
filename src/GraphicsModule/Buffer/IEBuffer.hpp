#pragma once

/* Predefine classes used with pointers or as return values for functions. */
class IEImage;

class IERenderEngine;

/* Include classes used as attributes or function arguments. */
// Internal dependencies
#include "GraphicsModule/CommandBuffer/IEDependency.hpp"
#include "IEAPI.hpp"

// External dependencies
#include <include/vk_mem_alloc.h>

#define GLEW_IMPLEMENTATION

#include <include/GL/glew.h>
#include <vulkan/vulkan.h>

// System dependencies
#include <cstdint>
#include <functional>
#include <vector>

class IEBuffer : public IEDependency, public std::enable_shared_from_this<IEBuffer> {
public:
    struct CreateInfo {
        size_t             size{};
        VkBufferUsageFlags usage{};
        VmaMemoryUsage     allocationUsage{};

        GLenum type{};
    };

    using IEBufferStatus = enum IEBufferStatus {
        IE_BUFFER_STATUS_NONE         = 0x0,
        IE_BUFFER_STATUS_UNLOADED     = 0x1,
        IE_BUFFER_STATUS_QUEUED_RAM   = 0x2,
        IE_BUFFER_STATUS_DATA_IN_RAM  = 0x4,
        IE_BUFFER_STATUS_QUEUED_VRAM  = 0x8,
        IE_BUFFER_STATUS_DATA_IN_VRAM = 0x10
    };

    std::vector<char>                  data{};
    VkBuffer                           buffer{};
    VkDeviceAddress                    deviceAddress{};
    size_t                             size{};
    VkBufferUsageFlags                 usage{};
    VmaMemoryUsage                     allocationUsage{};
    std::vector<std::function<void()>> deletionQueue{};
    IEBufferStatus                     status{IE_BUFFER_STATUS_NONE};
    GLuint                             id{};
    GLenum                             type{};

private:
    static std::function<void(IEBuffer &)> _uploadToRAM;

    static std::function<void(IEBuffer &)>                            _uploadToVRAM;
    static std::function<void(IEBuffer &, const std::vector<char> &)> _uploadToVRAM_vector;
    static std::function<void(IEBuffer &, void *, size_t)>            _uploadToVRAM_void;

    static std::function<void(IEBuffer &, const std::vector<char> &)> _update_vector;
    static std::function<void(IEBuffer &, void *, size_t)>            _update_void;

    static std::function<void(IEBuffer &)> _unloadFromVRAM;

    static std::function<void(IEBuffer &)> _destroy;

protected:
    virtual void _openglUploadToRAM();

    virtual void _vulkanUploadToRAM();


    virtual void _openglUploadToVRAM();

    virtual void _vulkanUploadToVRAM();

    virtual void _openglUploadToVRAM_vector(const std::vector<char> &);

    virtual void _vulkanUploadToVRAM_vector(const std::vector<char> &);

    virtual void _openglUploadToVRAM_void(void *, size_t);

    virtual void _vulkanUploadToVRAM_void(void *, size_t);


    virtual void _openglUpdate_vector(const std::vector<char> &);

    virtual void _vulkanUpdate_vector(const std::vector<char> &);

    virtual void _openglUpdate_void(void *, size_t);

    virtual void _vulkanUpdate_void(void *, size_t);


    virtual void _openglUnloadFromVRAM();

    virtual void _vulkanUnloadFromVRAM();


    virtual void _openglDestroy();

    virtual void _vulkanDestroy();

public:
    IEBuffer();

    IEBuffer(IERenderEngine *, IEBuffer::CreateInfo *);

    IEBuffer(
      IERenderEngine    *engineLink,
      size_t             bufferSize,
      VkBufferUsageFlags usageFlags,
      VmaMemoryUsage     memoryUsage,
      GLenum             bufferType
    );


    static void setAPI(const IEAPI &API);


    void create(IERenderEngine *engineLink, IEBuffer::CreateInfo *createInfo);

    void create(
      IERenderEngine    *engineLink,
      size_t             bufferSize,
      VkBufferUsageFlags usageFlags,
      VmaMemoryUsage     memoryUsage,
      GLenum             bufferType
    );


    void toImage(const std::shared_ptr<IEImage> &image);


    void uploadToRAM();

    void uploadToRAM(const std::vector<char> &);

    void uploadToRAM(void *, size_t);


    virtual void uploadToVRAM();

    void uploadToVRAM(const std::vector<char> &);

    void uploadToVRAM(void *, size_t);


    void update(const std::vector<char> &);

    void update(void *, size_t);


    void unloadFromVRAM();


    void destroy();


    ~IEBuffer() override;

protected:
    IERenderEngine *linkedRenderEngine{};
    VmaAllocation   allocation{};
};

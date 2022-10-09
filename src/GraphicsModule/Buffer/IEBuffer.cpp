/* Include this file's header. */
#include "IEBuffer.hpp"

/* Include dependencies within this module. */
#include "IERenderEngine.hpp"

void IEBuffer::setAPI(const IEAPI &API) {
    if (API.name == IE_RENDER_ENGINE_API_NAME_OPENGL) {
        _uploadToRAM         = &IEBuffer::_openglUploadToRAM;
        _uploadToVRAM        = &IEBuffer::_openglUploadToVRAM;
        _uploadToVRAM_vector = &IEBuffer::_openglUploadToVRAM_vector;
        _uploadToVRAM_void   = &IEBuffer::_openglUploadToVRAM_void;
        _update_vector       = &IEBuffer::_openglUpdate_vector;
        _update_void         = &IEBuffer::_openglUpdate_void;
        _unloadFromVRAM      = &IEBuffer::_openglUnloadFromVRAM;
        _destroy             = &IEBuffer::_openglDestroy;
    } else if (API.name == IE_RENDER_ENGINE_API_NAME_VULKAN) {
        _uploadToRAM         = &IEBuffer::_vulkanUploadToRAM;
        _uploadToVRAM        = &IEBuffer::_vulkanUploadToVRAM;
        _uploadToVRAM_vector = &IEBuffer::_vulkanUploadToVRAM_vector;
        _uploadToVRAM_void   = &IEBuffer::_vulkanUploadToVRAM_void;
        _update_vector       = &IEBuffer::_vulkanUpdate_vector;
        _update_void         = &IEBuffer::_vulkanUpdate_void;
        _unloadFromVRAM      = &IEBuffer::_vulkanUnloadFromVRAM;
        _destroy             = &IEBuffer::_vulkanDestroy;
    }
}

void IEBuffer::uploadToRAM(void *pData, size_t dataSize) {
    data = std::vector<char>{(char *) pData, (char *) ((uint64_t) pData + dataSize)};
    if (!data.empty()) status = static_cast<IEBufferStatus>(IE_BUFFER_STATUS_DATA_IN_RAM | status);
}

void IEBuffer::uploadToRAM(const std::vector<char> &dataVector) {
    data   = dataVector;
    status = static_cast<IEBufferStatus>(IE_BUFFER_STATUS_DATA_IN_RAM | status);
}

std::function<void(IEBuffer &)> IEBuffer::_uploadToRAM;

void IEBuffer::uploadToRAM() {
    _uploadToRAM(*this);
}

void IEBuffer::_openglUploadToRAM() {
    /**@todo Load from VRAM to RAM somehow.*/
}

void IEBuffer::_vulkanUploadToRAM() {
    void *internalBufferData;
    vmaMapMemory(linkedRenderEngine->allocator, allocation, &internalBufferData);
    memcpy(data.data(), internalBufferData, size);
    vmaUnmapMemory(linkedRenderEngine->allocator, allocation);
}

void IEBuffer::toImage(const std::shared_ptr<IEImage> &image) {
    VkBufferImageCopy region{};
    region.imageSubresource.aspectMask = image->aspect & (VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_DEPTH_BIT);
    region.imageSubresource.layerCount = 1;
    region.imageExtent                 = {image->width, image->height, 1};
    if (image->layout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        VkImageLayout oldLayout = image->layout;
        image->transitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        linkedRenderEngine->graphicsCommandPool->index(0)
          ->recordCopyBufferToImage(shared_from_this(), image, {region});
        if (oldLayout != VK_IMAGE_LAYOUT_UNDEFINED) image->transitionLayout(oldLayout);
    } else {
        linkedRenderEngine->graphicsCommandPool->index(0)
          ->recordCopyBufferToImage(shared_from_this(), image, {region});
    }
}

IEBuffer::~IEBuffer() {
    destroy();
}

IEBuffer::IEBuffer(IERenderEngine *engineLink, IEBuffer::CreateInfo *createInfo) {
    create(engineLink, createInfo);
}

IEBuffer::IEBuffer(
  IERenderEngine    *engineLink,
  size_t             bufferSize,
  VkBufferUsageFlags usageFlags,
  VmaMemoryUsage     memoryUsage,
  GLenum             bufferType
) {
    create(engineLink, bufferSize, usageFlags, memoryUsage, bufferType);
}

void IEBuffer::create(IERenderEngine *engineLink, IEBuffer::CreateInfo *createInfo) {
    if (status == IE_BUFFER_STATUS_NONE) {
        linkedRenderEngine = engineLink;
        size               = createInfo->size;
        usage              = createInfo->usage;
        allocationUsage    = createInfo->allocationUsage;
        type               = createInfo->type;
    }
    status = IE_BUFFER_STATUS_UNLOADED;
}

void IEBuffer::create(
  IERenderEngine    *engineLink,
  size_t             bufferSize,
  VkBufferUsageFlags usageFlags,
  VmaMemoryUsage     memoryUsage,
  GLenum             bufferType
) {
    if (status == IE_BUFFER_STATUS_NONE) {
        linkedRenderEngine = engineLink;
        size               = bufferSize;
        usage              = usageFlags;
        allocationUsage    = memoryUsage;
        type               = bufferType;
    }
    status = IE_BUFFER_STATUS_UNLOADED;
}

std::function<void(IEBuffer &)> IEBuffer::_uploadToVRAM = nullptr;

void IEBuffer::uploadToVRAM() {
    _uploadToVRAM(*this);
}

void IEBuffer::_openglUploadToVRAM() {
    if ((status & IE_BUFFER_STATUS_DATA_IN_RAM) == 0) {
        linkedRenderEngine->settings->logger.log(
          "Attempt to load buffer with no contents in RAM to VRAM.",
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_WARN
        );
    }
    if (!(status & IE_BUFFER_STATUS_DATA_IN_VRAM))  // Not in VRAM
        glGenBuffers(1, &id);                       // Put it in VRAM
    if (status & IE_BUFFER_STATUS_DATA_IN_RAM) {
        glBindBuffer(type, id);
        glBufferData(type, (GLsizeiptr) size, data.data(), GL_STATIC_DRAW);
        glBindBuffer(type, 0);
    }
}

void IEBuffer::_vulkanUploadToVRAM() {
    if ((status & IE_BUFFER_STATUS_DATA_IN_RAM) == 0) {
        linkedRenderEngine->settings->logger.log(
          "Attempt to load buffer with no contents in RAM to VRAM.",
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_WARN
        );
    }
    if ((status & IE_BUFFER_STATUS_DATA_IN_VRAM) == 0) {
        // Create the VkBuffer because it does not yet exist.
        VmaAllocationCreateInfo allocationCreateInfo{
          .usage = allocationUsage,
        };
        VkBufferCreateInfo bufferCreateInfo{
          .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
          .size  = size,
          .usage = usage,
        };

        // If usage requests device address, then prepare allocation for using device address
        if ((usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) != 0U)
            allocationCreateInfo.requiredFlags |= VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;

        // Create buffer and update status
        vmaCreateBuffer(
          linkedRenderEngine->allocator,
          &bufferCreateInfo,
          &allocationCreateInfo,
          &buffer,
          &allocation,
          nullptr
        );
        status = static_cast<IEBufferStatus>(IE_BUFFER_STATUS_DATA_IN_VRAM | status);

        // Get the buffer device address if requested
        if ((usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) != 0U) {
            VkBufferDeviceAddressInfoKHR bufferDeviceAddressInfo{VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO};
            bufferDeviceAddressInfo.buffer = buffer;
            deviceAddress                  = linkedRenderEngine->vkGetBufferDeviceAddressKHR(
              linkedRenderEngine->device.device,
              &bufferDeviceAddressInfo
            );
        }
    }

    // Upload data in RAM to VRAM
    void *internalBufferData;
    vmaMapMemory(linkedRenderEngine->allocator, allocation, &internalBufferData);
    memcpy(internalBufferData, data.data(), data.size());
    vmaUnmapMemory(linkedRenderEngine->allocator, allocation);
}

std::function<void(IEBuffer &, const std::vector<char> &)> IEBuffer::_uploadToVRAM_vector = nullptr;

void IEBuffer::uploadToVRAM(const std::vector<char> &data) {
    _uploadToVRAM_vector(*this, data);
}

void IEBuffer::_openglUploadToVRAM_vector(const std::vector<char> &data) {
    if (!(status & IE_BUFFER_STATUS_DATA_IN_VRAM)) {  // Not in VRAM
        glGenBuffers(1, &id);                         // Put it in VRAM
        size   = data.size();
        status = static_cast<IEBufferStatus>(status | IE_BUFFER_STATUS_DATA_IN_VRAM);
    }

    // Upload data
    glBindBuffer(type, id);
    glBufferData(type, (GLsizeiptr) data.size() * (GLsizeiptr) sizeof(data[0]), data.data(), GL_STATIC_DRAW);
    glBindBuffer(type, 0);
}

void IEBuffer::_vulkanUploadToVRAM_vector(const std::vector<char> &data) {
    if ((status & IE_BUFFER_STATUS_DATA_IN_VRAM) == 0) {
        // Create the VkBuffer because it does not yet exist.
        VmaAllocationCreateInfo allocationCreateInfo{
          .usage = allocationUsage,
        };
        VkBufferCreateInfo bufferCreateInfo{
          .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
          .size  = size,
          .usage = usage,
        };

        // If usage requests device address, then prepare allocation for using device address
        if ((usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) != 0U)
            allocationCreateInfo.requiredFlags |= VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;

        // Create buffer and update status
        vmaCreateBuffer(
          linkedRenderEngine->allocator,
          &bufferCreateInfo,
          &allocationCreateInfo,
          &buffer,
          &allocation,
          nullptr
        );
        status = static_cast<IEBufferStatus>(IE_BUFFER_STATUS_DATA_IN_VRAM | status);

        // Get the buffer device address if requested
        if ((usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) != 0U) {
            VkBufferDeviceAddressInfoKHR bufferDeviceAddressInfo{VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO};
            bufferDeviceAddressInfo.buffer = buffer;
            deviceAddress                  = linkedRenderEngine->vkGetBufferDeviceAddressKHR(
              linkedRenderEngine->device.device,
              &bufferDeviceAddressInfo
            );
        }
        status = static_cast<IEBufferStatus>(status | IE_BUFFER_STATUS_DATA_IN_VRAM);
    }

    // Upload data in RAM to VRAM
    void *internalBufferData;
    vmaMapMemory(linkedRenderEngine->allocator, allocation, &internalBufferData);
    memcpy(internalBufferData, data.data(), data.size());
    vmaUnmapMemory(linkedRenderEngine->allocator, allocation);
}

std::function<void(IEBuffer &, void *, size_t)> IEBuffer::_uploadToVRAM_void = nullptr;

void IEBuffer::uploadToVRAM(void *data, size_t size) {
    _uploadToVRAM_void(*this, data, size);
}

void IEBuffer::_openglUploadToVRAM_void(void *data, size_t size) {
    if (!(status & IE_BUFFER_STATUS_DATA_IN_VRAM)) {  // Not in VRAM?
        glGenBuffers(1, &id);                         // Put it in VRAM
        this->size = size;
        status     = static_cast<IEBufferStatus>(status | IE_BUFFER_STATUS_DATA_IN_VRAM);
    }

    // Upload data
    glBindBuffer(type, id);
    glBufferData(type, (GLsizeiptr) size, data, GL_STATIC_DRAW);
    glBindBuffer(type, 0);
}

void IEBuffer::_vulkanUploadToVRAM_void(void *data, size_t size) {
    if ((status & IE_BUFFER_STATUS_DATA_IN_VRAM) == 0) {
        // Create the VkBuffer because it does not yet exist.
        VmaAllocationCreateInfo allocationCreateInfo{
          .usage = allocationUsage,
        };
        VkBufferCreateInfo bufferCreateInfo{
          .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
          .size  = size,
          .usage = usage,
        };

        // If usage requests device address, then prepare allocation for using device address
        if ((usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) != 0U)
            allocationCreateInfo.requiredFlags |= VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;

        // Create buffer and update status
        vmaCreateBuffer(
          linkedRenderEngine->allocator,
          &bufferCreateInfo,
          &allocationCreateInfo,
          &buffer,
          &allocation,
          nullptr
        );
        status = static_cast<IEBufferStatus>(IE_BUFFER_STATUS_DATA_IN_VRAM | status);

        // Get the buffer device address if requested
        if ((usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) != 0U) {
            VkBufferDeviceAddressInfoKHR bufferDeviceAddressInfo{VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO};
            bufferDeviceAddressInfo.buffer = buffer;
            deviceAddress                  = linkedRenderEngine->vkGetBufferDeviceAddressKHR(
              linkedRenderEngine->device.device,
              &bufferDeviceAddressInfo
            );
        }

        this->size = size;
        status     = static_cast<IEBufferStatus>(status | IE_BUFFER_STATUS_DATA_IN_VRAM);
    }

    // Upload data in RAM to VRAM
    void *internalBufferData;
    vmaMapMemory(linkedRenderEngine->allocator, allocation, &internalBufferData);
    memcpy(internalBufferData, data, size);
    vmaUnmapMemory(linkedRenderEngine->allocator, allocation);
}

std::function<void(IEBuffer &, const std::vector<char> &)> IEBuffer::_update_vector = nullptr;

void IEBuffer::update(const std::vector<char> &data) {
    _update_vector(*this, data);
}

void IEBuffer::_openglUpdate_vector(const std::vector<char> &data) {
    if (status & IE_BUFFER_STATUS_DATA_IN_VRAM) {  // In VRAM?
        glBindBuffer(type, id);
        glBufferData(type, (GLsizeiptr) data.size() * (GLsizeiptr) sizeof(data[0]), data.data(), GL_STATIC_DRAW);
        glBindBuffer(type, 0);
    }
    if (status & IE_BUFFER_STATUS_DATA_IN_RAM)  // In RAM?
        this->data = data;
}

void IEBuffer::_vulkanUpdate_vector(const std::vector<char> &data) {
    if (status & IE_BUFFER_STATUS_DATA_IN_VRAM) {  // In VRAM?
        // Upload data in RAM to VRAM
        void *internalBufferData;
        vmaMapMemory(linkedRenderEngine->allocator, allocation, &internalBufferData);
        memcpy(internalBufferData, data.data(), data.size());
        vmaUnmapMemory(linkedRenderEngine->allocator, allocation);
    }
    if (status & IE_BUFFER_STATUS_DATA_IN_RAM)  // In RAM?
        this->data = data;
}

std::function<void(IEBuffer &, void *, size_t)> IEBuffer::_update_void = nullptr;

void IEBuffer::update(void *data, size_t size) {
    _update_void(*this, data, size);
}

void IEBuffer::_openglUpdate_void(void *data, size_t size) {
    if (status & IE_BUFFER_STATUS_DATA_IN_VRAM) {  // In VRAM?
        glBindBuffer(type, id);
        glBufferData(type, (GLsizeiptr) size, data, GL_STATIC_DRAW);
        glBindBuffer(type, 0);
    }
    if (status & IE_BUFFER_STATUS_DATA_IN_RAM)  // In RAM?
        this->data = std::vector<char>{(char *) data, (char *) ((size_t) data + size)};
}

void IEBuffer::_vulkanUpdate_void(void *data, size_t size) {
    if (status & IE_BUFFER_STATUS_DATA_IN_VRAM) {  // In VRAM?
        // Upload data in RAM to VRAM
        void *internalBufferData;
        vmaMapMemory(linkedRenderEngine->allocator, allocation, &internalBufferData);
        memcpy(internalBufferData, data, size);
        vmaUnmapMemory(linkedRenderEngine->allocator, allocation);
    }
    if (status & IE_BUFFER_STATUS_DATA_IN_RAM)  // In RAM?
        this->data = std::vector<char>{(char *) data, (char *) ((size_t) data + size)};
}

std::function<void(IEBuffer &)> IEBuffer::_unloadFromVRAM = nullptr;

void IEBuffer::unloadFromVRAM() {
    _unloadFromVRAM(*this);
}

void IEBuffer::_openglUnloadFromVRAM() {
    if (status & IE_BUFFER_STATUS_DATA_IN_VRAM) {  // In VRAM?
        glDeleteBuffers(1, &id);
        status = static_cast<IEBufferStatus>(status & ~IE_BUFFER_STATUS_DATA_IN_VRAM);  // Not in VRAM
    }
}

void IEBuffer::_vulkanUnloadFromVRAM() {
    vmaDestroyBuffer(linkedRenderEngine->allocator, buffer, allocation);
    status = static_cast<IEBufferStatus>(status & ~IE_BUFFER_STATUS_DATA_IN_VRAM);
}

std::function<void(IEBuffer &)> IEBuffer::_destroy = nullptr;

void IEBuffer::destroy() {
    invalidateDependents();
    _destroy(*this);
}

void IEBuffer::_openglDestroy() {
    if (status & IE_BUFFER_STATUS_DATA_IN_VRAM) {  // In VRAM?
        glDeleteBuffers(1, &id);
        status =
          static_cast<IEBufferStatus>(status & ~IE_BUFFER_STATUS_DATA_IN_VRAM | IE_BUFFER_STATUS_QUEUED_VRAM);
    }
    if (status & IE_BUFFER_STATUS_DATA_IN_RAM) {  // In RAM?
        data.clear();
        status = static_cast<IEBufferStatus>(
          status & ~IE_BUFFER_STATUS_DATA_IN_RAM & ~IE_BUFFER_STATUS_QUEUED_VRAM | IE_BUFFER_STATUS_QUEUED_RAM
        );
    }
}

void IEBuffer::_vulkanDestroy() {
    if (status & IE_BUFFER_STATUS_DATA_IN_VRAM) {  // In VRAM?
        vmaDestroyBuffer(linkedRenderEngine->allocator, buffer, allocation);
        status =
          static_cast<IEBufferStatus>(status & ~IE_BUFFER_STATUS_DATA_IN_VRAM | IE_BUFFER_STATUS_QUEUED_VRAM);
    }
    if (status & IE_BUFFER_STATUS_DATA_IN_RAM) {  // In RAM?
        data.clear();
        status = static_cast<IEBufferStatus>(
          status & ~IE_BUFFER_STATUS_DATA_IN_RAM & ~IE_BUFFER_STATUS_QUEUED_VRAM | IE_BUFFER_STATUS_QUEUED_RAM
        );
    }
}

IEBuffer::IEBuffer() = default;

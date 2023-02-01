#include "BufferVulkan.hpp"

#include "RenderEngine.hpp"

bool IE::Graphics::detail::BufferVulkan::_createBuffer(
  Type     t_type,
  uint64_t t_flags,
  void    *t_data,
  size_t   t_dataSize
) {
    std::unique_lock<std::mutex> lock(*m_mutex);
    VkBufferCreateInfo           createInfo{
                .sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                .pNext                 = nullptr,
                .flags                 = 0,
                .size                  = t_dataSize,
                .usage                 = VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                .sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
                .queueFamilyIndexCount = 0,
                .pQueueFamilyIndices   = nullptr};
    VmaAllocationCreateInfo allocationCreateInfo{
      .flags          = 0x0,
      .usage          = VMA_MEMORY_USAGE_AUTO,
      .requiredFlags  = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
      .preferredFlags = 0x0,
      .memoryTypeBits = 0x0,
      .pool           = VK_NULL_HANDLE,
      .pUserData      = nullptr,
      .priority       = 0x0};
    VkResult result{vmaCreateBuffer(
      m_linkedRenderEngine->getAllocator(),
      &createInfo,
      &allocationCreateInfo,
      &m_buffer,
      &m_allocation,
      &m_allocationInfo
    )};
    if (result != VK_SUCCESS) {
        m_linkedRenderEngine->getLogger().log(
          "Failed to create buffer with error: " + IE::Graphics::RenderEngine::translateVkResultCodes(result)
        );
        return false;
    }
    void *ppData{};
    vmaMapMemory(m_linkedRenderEngine->getAllocator(), m_allocation, &ppData);
    memcpy(ppData, t_data, t_dataSize);
    vmaUnmapMemory(m_linkedRenderEngine->getAllocator(), m_allocation);
    m_linkedRenderEngine->getLogger().log("Created buffer");
    return true;
}

bool IE::Graphics::detail::BufferVulkan::_destroyBuffer() {
    vmaDestroyBuffer(m_linkedRenderEngine->getAllocator(), m_buffer, m_allocation);
    return true;
}

GLenum IE::Graphics::detail::BufferVulkan::getGLBuffer() {
    throw std::runtime_error("Invalid API!");
}

VkBuffer IE::Graphics::detail::BufferVulkan::getVkBuffer() {
    return m_buffer;
}

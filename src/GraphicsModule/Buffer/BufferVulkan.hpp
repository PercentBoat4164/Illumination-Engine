#pragma once

#include "Buffer.hpp"

#include <vk_mem_alloc.h>

namespace IE::Graphics::detail {
class BufferVulkan : public ::IE::Graphics::Buffer {
public:
    VkBuffer           m_id{};
    VkBufferUsageFlags m_usage{};
    VmaMemoryUsage     m_allocationUsage{};
    VmaAllocationInfo  m_allocationInfo{};
    VmaAllocation      m_allocation{};

protected:
    void _createBuffer(Type type, uint64_t flags, void *data, size_t dataSize) override;

    void _destroyBuffer() override;
};
}  // namespace IE::Graphics::detail
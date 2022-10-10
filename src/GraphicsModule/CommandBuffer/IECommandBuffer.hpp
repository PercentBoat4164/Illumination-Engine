#pragma once

class IECommandPool;

class IERenderEngine;

#include "Buffer/IEBuffer.hpp"
#include "Image/Image.hpp"
#include "GraphicsModule/Shader/IEPipeline.hpp"
#include "CommandBuffer/DependencyStructs/IEBufferMemoryBarrier.hpp"
#include "CommandBuffer/DependencyStructs/IEImageMemoryBarrier.hpp"
#include "CommandBuffer/DependencyStructs/IEDependencyInfo.hpp"
#include "CommandBuffer/DependencyStructs/IECopyBufferToImageInfo.hpp"
#include "CommandBuffer/DependencyStructs/IEDependencyInfo.hpp"
#include "CommandBuffer/DependencyStructs/IERenderPassBeginInfo.hpp"
#include "GraphicsModule/Shader/IEPipeline.hpp"
#include "IEDependent.hpp"
#include "Image/IEImage.hpp"

#include <mutex>
#include <thread>
#include <variant>
#include <vector>
#include <vulkan/vulkan.h>

typedef enum IECommandBufferStatus {
    IE_COMMAND_BUFFER_STATE_NONE       = 0x0,
    IE_COMMAND_BUFFER_STATE_INITIAL    = 0x1,
    IE_COMMAND_BUFFER_STATE_RECORDING  = 0x2,
    IE_COMMAND_BUFFER_STATE_EXECUTABLE = 0x3,
    IE_COMMAND_BUFFER_STATE_PENDING    = 0x4,
    IE_COMMAND_BUFFER_STATE_INVALID    = 0x5
} IECommandBufferStatus;

class IECommandBuffer {
public:
	VkCommandBuffer commandBuffer{};
	std::shared_ptr<IECommandPool> commandPool{};
	IERenderEngine *linkedRenderEngine{};
	IECommandBufferStatus status{};
	bool oneTimeSubmission{false};
	std::thread executionThread{};

	IECommandBuffer(IERenderEngine *linkedRenderEngine, const std::shared_ptr<IECommandPool> &parentCommandPool);

	void wait();

	/**
	 * @brief Allocate this command buffer as a primary command buffer.
	 */
	void allocate(bool synchronize = true);

	/**
	 * @brief Prepare this command buffer for recording.
	 */
	void record(bool synchronize = true, bool oneTimeSubmit = false);

	void free(bool synchronize = true);

	void reset(bool synchronize = true);

	void execute(VkSemaphore input = (VkSemaphore) (void *) nullptr, VkSemaphore output = (VkSemaphore) (void *) nullptr,
				 VkFence fence = (VkFence) (void *) nullptr
	);
	void finish(bool synchronize = true);

	void recordPipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
							   const std::vector<VkMemoryBarrier> &memoryBarriers, const std::vector<IEBufferMemoryBarrier> &bufferMemoryBarriers,
							   const std::vector<IEImageMemoryBarrier> &imageMemoryBarriers
	);
	void recordPipelineBarrier(const IEDependencyInfo *dependencyInfo);

	voidrecordCopyBufferToImage(
      const std::shared_ptr<IEBuffer> &buffer,
      const std::shared_ptr<IE::Graphics::Image> &image,
							std::vector<VkBufferImageCopy> regions
	);
	void recordCopyBufferToImage(IECopyBufferToImageInfo *copyInfo);

	voidrecordBindVertexBuffers(
      uint32_t                               firstBinding,
      uint32_t                               bindingCount,
      std::vector<std::shared_ptr<IEBuffer>> buffers,
      VkDeviceSize                          *pOffsets
	);
	void recordBindVertexBuffers(uint32_t firstBinding, uint32_t bindingCount, const std::vector<std::shared_ptr<IEBuffer>> &buffers,
								 VkDeviceSize *pOffsets, VkDeviceSize *pSizes, VkDeviceSize *pStrides);

	void recordBindPipeline(VkPipelineBindPoint pipelineBindPoint, const std::shared_ptr<IEPipeline> &pipeline);

	void recordBindDescriptorSets(VkPipelineBindPoint pipelineBindPoint, const std::shared_ptr<IEPipeline> &pipeline, uint32_t firstSet,
								  const std::vector<std::shared_ptr<IEDescriptorSet>> &descriptorSets, std::vector<uint32_t> dynamicOffsets
	);
	void recordDrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);

	void recordBindIndexBuffer(const std::shared_ptr<IEBuffer> &buffer, uint32_t offset, VkIndexType indexType);

	void recordBeginRenderPass(IERenderPassBeginInfo *pRenderPassBegin, VkSubpassContents contents);

	void recordSetViewport(uint32_t firstViewPort, uint32_t viewPortCount, const VkViewport *pViewPorts);

	void recordSetScissor(uint32_t firstScissor, uint32_t scissorCount, const VkRect2D *pScissors);

	void recordEndRenderPass();

	void destroy();

	~IECommandBuffer();

	IECommandBuffer(const IECommandBuffer &source) = delete;

	IECommandBuffer(IECommandBuffer &&source) noexcept {};
};
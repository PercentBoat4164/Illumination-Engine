#pragma once

/* Predefine classes used with pointers or as return values for functions. */
class IEImage;

class IERenderEngine;

/* Include classes used as attributes or function arguments. */
// Internal dependencies
#include "GraphicsModule/CommandBuffer/IEDependency.hpp"

// External dependencies
#include <vk_mem_alloc.h>
#include <GL/glew.h>

#include <vulkan/vulkan.h>

// System dependencies
#include <vector>
#include <cstdint>
#include <functional>


class IEBuffer : public IEDependency, public std::enable_shared_from_this<IEBuffer> {
public:
	struct CreateInfo {
		VkDeviceSize size{};
		VkBufferUsageFlags usage{};
		VmaMemoryUsage allocationUsage{};
	};

	typedef enum IEBufferStatus {
		IE_BUFFER_STATUS_NONE = 0x0,
		IE_BUFFER_STATUS_UNLOADED = 0x1,
		IE_BUFFER_STATUS_QUEUED_RAM = 0x2,
		IE_BUFFER_STATUS_DATA_IN_RAM = 0x3,
		IE_BUFFER_STATUS_QUEUED_VRAM = 0x4,
		IE_BUFFER_STATUS_DATA_IN_VRAM = 0x5
	} IEBufferStatus;

	std::vector<char> data{};
	VkBuffer buffer{};
	VkDeviceAddress deviceAddress{};
	VkDeviceSize size{};
	VkBufferUsageFlags usage{};
	VmaMemoryUsage allocationUsage{};
	std::vector<std::function<void()>> deletionQueue{};
	IEBufferStatus status{IE_BUFFER_STATUS_NONE};
	GLuint bufferID{};

private:
	static std::function<void(IEImage &)> _uploadToVRAM;
	static std::function<void(IEImage &, const std::vector<char> &)> _uploadToVRAM_vector;
	static std::function<void(IEImage &, void *, std::size_t)> _uploadToVRAM_void;
	
	static std::function<void(IEImage &, const std::vector<char> &)> _update_vector;
	static std::function<void(IEImage &, void *, std::size_t)> _update_void;
	
	static std::function<void(IEImage &)> _unloadFromVRAM;
	
	static std::function<void(IEImage &)> _destroy;
	
protected:
	virtual void _openglUploadToVRAM();
	
	virtual void _vulkanUploadToVRAM();
	
	virtual void _openglUploadToVRAM_vector(const std::vector<char> &);
	
	virtual void _vulkanUploadToVRAM_vector(const std::vector<char> &);
	
	virtual void _openglUploadToVRAM_void(void *, std::size_t);
	
	virtual void _vulkanUploadToVRAM_void(void *, std::size_t);
	
	
	virtual void _openglUpdate_vector(const std::vector<char> &);
	
	virtual void _vulkanUpdate_vector(const std::vector<char> &);
	
	virtual void _openglUpdate_voidPtr(void *, std::size_t);
	
	virtual void _vulkanUpdate_voidPtr(void *, std::size_t);
	
	
	virtual void _openglUnloadFromVRAM();
	
	virtual void _vulkanUnloadFromVRAM();
	
	
	virtual void _openglDestroy();
	
	virtual void _vulkanDestroy();
	
public:
	void destroy(bool= false);

	IEBuffer();

	IEBuffer(IERenderEngine *, IEBuffer::CreateInfo *);

	IEBuffer(IERenderEngine *, VkDeviceSize, VkBufferUsageFlags, VmaMemoryUsage);

	void create(IERenderEngine *engineLink, IEBuffer::CreateInfo *createInfo);
	
	void create(IERenderEngine *, VkDeviceSize, VkBufferUsageFlags, VmaMemoryUsage);
	
	void toImage(const std::shared_ptr<IEImage> &image);

	~IEBuffer() override;
	
	void uploadToRAM();

	void loadFromRAMToVRAM();

	void loadFromDiskToRAM(void *, uint32_t);

	void loadFromDiskToRAM(const std::vector<char> &);

	void unloadFromVRAM();

protected:
	IERenderEngine *linkedRenderEngine{};
	VmaAllocation allocation{};
};

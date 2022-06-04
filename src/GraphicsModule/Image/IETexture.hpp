#pragma once

/* Predefine classes used with pointers or as return values for functions. */
class IEBuffer;

/* Include classes used as attributes or function arguments. */
// Internal dependencies
#include "IEImage.hpp"

// External dependencies
#include <assimp/material.h>
#include <vulkan/vulkan.h>

#include <stb_image.h>

// System dependencies
#include <cstdint>
#include <string>

class aiTexture;

class IETexture : public IEImage {
public:
	struct CreateInfo {
		VkFormat format{VK_FORMAT_R8G8B8A8_SRGB};
		VkImageLayout layout{VK_IMAGE_LAYOUT_UNDEFINED};
		VkImageType type{VK_IMAGE_TYPE_2D};
		VkImageUsageFlags usage{VK_IMAGE_USAGE_SAMPLED_BIT};
		VkImageCreateFlags flags{};
		VmaMemoryUsage allocationUsage{};
	};

	IETexture() = default;

	IETexture(IERenderEngine *, IETexture::CreateInfo *);

	static void setAPI(const IEAPI &);


	void create(IERenderEngine *, IETexture::CreateInfo *);

private:
	static std::function<void(IETexture &)> _uploadToVRAM;

	static std::function<void(IETexture &, const std::vector<char> &)> _update_vector;
	static std::function<void(IETexture &, void *, uint64_t)> _update_voidPtr;
	static std::function<void(IETexture &, aiTexture *)> _update_aiTexture;

	static std::function<void(IETexture &)> _unloadFromVRAM;

	static std::function<void(IETexture &)> _destroy;

protected:
	void _openglUploadToVRAM() override;

	void _vulkanUploadToVRAM() override;


	virtual void _openglUpdate_aiTexture(aiTexture *);

	virtual void _vulkanUpdate_aiTexture(aiTexture *);

public:
	void uploadToVRAM() override;

	void uploadToVRAM(const std::vector<char> &) override;

	void uploadToVRAM(void *, uint64_t) override;

	void uploadToVRAM(aiTexture *texture);


	void update(const std::vector<char> &) override;

	void update(void *, uint64_t) override;

	virtual void update(aiTexture *);


	void unloadFromVRAM() override;


	void destroy() override;
};
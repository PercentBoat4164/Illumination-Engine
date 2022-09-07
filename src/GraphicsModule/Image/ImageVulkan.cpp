#include "ImageVulkan.hpp"

#include "utility"

IE::ImageVulkan::ImageVulkan() : Image{},
								 format{VK_FORMAT_UNDEFINED},
								 layout{VK_IMAGE_LAYOUT_UNDEFINED},
								 type{VK_IMAGE_TYPE_1D},
								 tiling{VK_IMAGE_TILING_OPTIMAL},
								 usage{VK_IMAGE_USAGE_TRANSFER_SRC_BIT},
								 flags{0},
								 aspect{VK_IMAGE_ASPECT_NONE} {}

IE::ImageVulkan::ImageVulkan(IE::ImageVulkan &&other) noexcept: Image{dynamic_cast<Image &&>(other)},
																format{std::exchange(other.format, VK_FORMAT_UNDEFINED)},
																layout{std::exchange(other.layout, VK_IMAGE_LAYOUT_UNDEFINED)},
																type{std::exchange(other.type, VK_IMAGE_TYPE_1D)},
																tiling{std::exchange(other.tiling, VK_IMAGE_TILING_OPTIMAL)},
																usage{std::exchange(other.usage, VK_IMAGE_USAGE_TRANSFER_SRC_BIT)},
																flags{std::exchange(other.flags, 0)},
																aspect{std::exchange(other.aspect, VK_IMAGE_ASPECT_NONE)} {}

IE::ImageVulkan::ImageVulkan(const IE::ImageVulkan &other) : Image{dynamic_cast<const Image &>(other)},
															 format{other.format},
															 layout{other.layout},
															 type{other.type},
															 tiling{other.tiling},
															 usage{other.usage},
															 flags{other.flags},
															 aspect{other.aspect} {}

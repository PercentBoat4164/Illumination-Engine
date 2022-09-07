#pragma once

#include "IERenderEngine.hpp"
#include <cstdint>

namespace IE { class Image; }

class IE::Image {
public:
	typedef enum Location {
		IE_IMAGE_LOCATION_NULL = 0x0,
		IE_IMAGE_LOCATION_NONE = 0x1,
		IE_IMAGE_LOCATION_SYSTEM = 0x2,
		IE_IMAGE_LOCATION_VIDEO = 0x4
	} Location;
	
	
	uint32_t width{};  // Image width given in pixels
	uint32_t height{};  // Image height given in pixels
	uint32_t channels{};  // Image channel count, including any alpha channels
	std::weak_ptr<IERenderEngine> linkedRenderEngine{};  // A pointer to the IE::RenderEngine that controls this pointer
	
	/** @brief Default constructor as it is needed by classes that inherit from IE::Image. **/
	Image() = default;
	
	/** @brief Default destructor as it is used by classes that inherit from IE::Image. **/
	~Image() = default;
	
	/** @brief Defined move constructor to move over all base level image data to the new IE::Image being created and delete it from the original one.
	 * @param other IE::Image to move from and return to default values.
	 **/
	Image(Image &&other) noexcept : width(std::exchange(other.width, 0)), height(std::exchange(other.height, 0)), channels(std::exchange(other.channels, 0)), linkedRenderEngine(std::exchange(other.linkedRenderEngine, std::weak_ptr<IERenderEngine>())) {}
	
	/** @brief Defined copy constructor to copy over all base level image data to the new IE::Image being created.
	 * @param other IE::Image to copy from.
	 **/
	Image(const Image &other) noexcept = default;
	
	/** @brief Defined copy assignment operator to allow usage of '=' for reassignment of the IE::Image.
	 * @details During this assignment, no objects are deleted from either IE::Image. The linkedRenderEngine, though being a pointer, is not destroyed as it is probably also being pointed to by another object. Likely the IE::Image being copied into it.
	 * @param other IE::Image to copy from.
	 **/
	 Image &operator=(const Image &other) {
		 if (&other != this) {
			width = other.width;
			height = other.height;
			channels = other.channels;
			if (linkedRenderEngine.lock() != other.linkedRenderEngine.lock()) {
				/**@todo Add the parts of this function that would destroy the image and rebuild it under the new engine. This may have to be implemented in the classes that inherit this one. */
				linkedRenderEngine = other.linkedRenderEngine;
			}
		 }
		 return *this;
	 }
	 
	 Image &operator=(Image &&other) noexcept {
		 if (&other != this) {
			 width = std::exchange(other.width, 0);
			 height = std::exchange(other.height, 0);
			 channels = std::exchange(other.channels, 0);
			 if (linkedRenderEngine.lock() != other.linkedRenderEngine.lock()) {
				 /**@todo Add the parts of this function that would destroy the image and rebuild it under the new engine. This may have to be implemented in the classes that inherit this one. */
				 linkedRenderEngine = other.linkedRenderEngine;
			 }
		 }
		 return *this;
	 }
};
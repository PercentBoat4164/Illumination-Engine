#pragma once

#include "IERenderEngine.hpp"
#include "Core/MultiDimensionalVector.hpp"

#include "stb_image.h"

#include <cstdint>
#include <memory>
#include <vector>

namespace IE { class Image; }

class IE::Image {
public:
	typedef enum Location {
		IE_IMAGE_LOCATION_NULL = 0x0,
		IE_IMAGE_LOCATION_NONE = 0x1,
		IE_IMAGE_LOCATION_SYSTEM = 0x2,
		IE_IMAGE_LOCATION_VIDEO = 0x4
	} Location;
	
	size_t m_components;  // The number of channels in each pixel.
	Location m_location;  // The location(s) that the image is stored in.
	IE::Core::MultiDimensionalVector<unsigned char> m_data;  // The location that image data is stored on the CPU.
	std::weak_ptr<IERenderEngine> m_linkedRenderEngine;  // A pointer to the IE::RenderEngine that controls this pointer.
	
	/** @brief A factory function that creates an IE::Image from the render engine provided.
	 * @details Creates a blank IE::Image of no particular size or shape. The only defining characteristic of the created image is its link to the
	 * render engine.
	 * @param t_engineLink
	 * @return A std::unique_ptr<IE::Image>
	 */
	static std::unique_ptr<IE::Image> create(const std::weak_ptr<IERenderEngine>& t_engineLink);
	
	explicit Image(const std::weak_ptr<IERenderEngine> &t_engineLink);
	
	/** @brief This constructor allows the user to specify a multi-dimensional image.
	 * @details The uses of the multi-dimensionality that this constructor enables may include 1D, 2D, or 3D images which can be used in a graphics
	 * API. It may also include any number of dimensions for 4D noise textures, or any other multi-dimensional image requirement.
	 * @param t_dimensions A parameter pack of unsigned integers dictating the dimensionality of the image.
	 * @return This IE::Image
	 */
	template<typename... Args> explicit Image(Args... t_dimensions);
	
	/** @brief Default constructor as it is needed by classes that inherit from IE::Image.
	 * @return This IE::Image
	 * */
	Image() noexcept;

	/** @brief Default destructor as it is used by classes that inherit from IE::Image.
	 * @return This IE::Image
	 * */
	virtual ~Image() = default;
	
	/** @brief Defined move constructor to move over all base level image data to the new IE::Image being created and delete it from the original one.
	 * @param other IE::Image to move from and return to default values.
	 * @return This IE::Image
	 **/
	Image(Image &&other) noexcept = default;
	
	/** @brief Defined copy constructor to copy over all base level image data to the new IE::Image being created.
	 * @param IE::Image IE::Image to copy from.
	 * @return This IE::Image
	 **/
	Image(const Image &) = default;

	 /** @brief The move assignment operator must be overloaded because the moved image should not take on the renderEngineLink of the move source.
	  * @details This operation will work with two images from different IERenderEngines. In that case, neither image will change the render engine it
	  * works with the image being assigned will move the data to itself, and therefore to under the control of its render engine.
	  * @param other IE::Image to move from
	  * @return This IE::Image
	  */
	 Image &operator=(Image &&other) noexcept;
	
	/** @brief The copy assignment operator must be overridden because copied image should not copy the renderEngineLink of their original.
	 * @details This operation will work with two images from different IERenderEngines. In that case, neither image will change the render engine it
	 * works with. The image being assigned will copy the data from the other image into itself despite the difference in controlling render engines.
	 * @param other IE::Image to copy from.
	 * @return This IE::Image
	 **/
	Image &operator=(const Image &other);
	
	/** @brief [] operator support for images.
	 * @details Takes a number of arguments equal to the number of dimensions in the image. Supports Python-like negative indexing.
	 * @param args Any number of signed integers.
	 * @return The pixel value located at the given index.
	 */
	template<typename ...Args> unsigned char operator[](Args... args);
	
	[[nodiscard]] virtual uint8_t getBytesInFormat() const = 0;
	
	virtual void setLocation(Location) = 0;
	
	[[nodiscard]] Location getLocation() const;
	
	virtual void setData(const IE::Core::MultiDimensionalVector<unsigned char> &) = 0;
	
	[[nodiscard]] IE::Core::MultiDimensionalVector<unsigned char> getData() const;
	
	
};
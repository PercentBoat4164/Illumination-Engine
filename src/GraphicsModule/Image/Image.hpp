#pragma once

class IERenderEngine;

#include "Core/MultiDimensionalVector.hpp"

#include "stb_image.h"

#include <cstdint>
#include <memory>
#include <vector>
#include <mutex>


///@todo Add static create functions for all constructors.
namespace IE::Graphics {
	class Image {
	public:
		enum Location {
			IE_IMAGE_LOCATION_NULL = 0x0,
			IE_IMAGE_LOCATION_NONE = 0x1,
			IE_IMAGE_LOCATION_SYSTEM = 0x2,
			IE_IMAGE_LOCATION_VIDEO = 0x4
		};
		
		size_t m_components;  // The number of channels in each pixel.
		Location m_location;  // The location(s) that the image is stored in.
		std::vector<size_t> m_dimensions;
		IE::Core::MultiDimensionalVector<unsigned char> m_data;  // The location that image data is stored on the CPU.
		std::weak_ptr<IERenderEngine> m_linkedRenderEngine;  // A pointer to the IE::RenderEngine that controls this pointer.
		std::shared_ptr<std::mutex> m_mutex;
		
		/** @brief A factory function that creates an IE::Graphics::Image from the render engine provided.
		 * @details Creates a blank IE::Graphics::Image of no particular size or shape. The only defining characteristic of the created image is its
		 * link to the render engine.
		 * @param t_engineLink
		 * @return A std::unique_ptr<IE::Graphics::Image>
		 */
		static std::unique_ptr<IE::Graphics::Image> create(const std::weak_ptr<IERenderEngine> &t_engineLink);
		
		explicit Image(const std::weak_ptr<IERenderEngine> &t_engineLink);
		
		/** @brief This constructor allows the user to specify a multi-dimensional image.
		 * @details The uses of the multi-dimensionality that this constructor enables may include 1D, 2D, or 3D images which can be used in a
		 * graphics API. It may also include any number of dimensions for 4D noise textures, or any other multi-dimensional image requirement.
		 * @param t_dimensions A parameter pack of unsigned integers dictating the dimensionality of the image.
		 * @return This IE::Graphics::Image
		 */
		template<typename... Args>
		explicit Image(Args... t_dimensions);
		
		/** @brief Default constructor as it is needed by classes that inherit from IE::Graphics::Image.
		 * @return This IE::Graphics::Image
		 * */
		Image() noexcept;
		
		/** @brief Default destructor as it is used by classes that inherit from IE::Graphics::Image.
		 * @return This IE::Graphics::Image
		 * */
		virtual ~Image() = default;
		
		/** @brief Defined move constructor to move over all base level image data to the new IE::Graphics::Image being created and delete it from the
		 * original one.
		 * @param other IE::Graphics::Image to move from and return to default values.
		 * @return This IE::Graphics::Image
		 **/
		Image(Image &&other) noexcept = default;
		
		/** @brief Defined copy constructor to copy over all base level image data to the new IE::Graphics::Image being created.
		 * @param IE::Graphics::Image IE::Graphics::Image to copy from.
		 * @return This IE::Graphics::Image
		 **/
		Image(const Image &) = default;
		
		/** @brief The move assignment operator must be overloaded because the moved image should not take on the renderEngineLink of the move source.
		 * @details This operation will work with two images from different IERenderEngines. In that case, neither image will change the render engine
		 * it works with the image being assigned will move the data to itself, and therefore to under the control of its render engine.
		 * @param t_other IE::Graphics::Image to move from
		 * @return This IE::Graphics::Image
		 */
		Image &operator=(Image &&t_other) noexcept;
		
		/** @brief The copy assignment operator must be overridden because copied image should not copy the renderEngineLink of their original.
		 * @details This operation will work with two images from different IERenderEngines. In that case, neither image will change the render engine
		 * it works with. The image being assigned will copy the data from the other image into itself despite the difference in controlling render
		 * engines.
		 * @param other IE::Graphics::Image to copy from.
		 * @return This IE::Graphics::Image
		 **/
		Image &operator=(const Image &t_other);
		
		/** @brief [] operator support for images.
		 * @details Takes a number of arguments equal to the number of dimensions in the image. Supports Python-like negative indexing.
		 * @param args Any number of signed integers.
		 * @return The pixel value located at the given index.
		 */
		template<typename ...Args>
		unsigned char operator[](Args... args);
		
		[[nodiscard]] virtual uint8_t getBytesInFormat() const = 0;
		
		virtual void setLocation(Location) = 0;
		
		[[nodiscard]] Location getLocation() const;
		
		virtual void setData(const IE::Core::MultiDimensionalVector<unsigned char> &);
		
		[[nodiscard]] IE::Core::MultiDimensionalVector<unsigned char> getData() const;
	
	protected:
		virtual bool _createImage(const IE::Core::MultiDimensionalVector<unsigned char> &t_data) = 0;
		
		virtual void _getImageData(IE::Core::MultiDimensionalVector<unsigned char> *t_pData) const = 0;
		
		virtual void _setImageData(const IE::Core::MultiDimensionalVector<unsigned char> &t_data) = 0;
		
		virtual void _destroyImage() = 0;
	};
}

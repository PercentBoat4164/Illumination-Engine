#include "IENDimensionalResizableImage.hpp"

char *IENDimensionalResizableImage::getData() const {
    return m_data;
}

size_t IENDimensionalResizableImage::getSize() const {
    return m_size;
}

size_t *IENDimensionalResizableImage::getDimensions() const {
    return m_dimensions;
}

size_t IENDimensionalResizableImage::getDimensionCount() const {
    return m_dimensionCount;
}

size_t IENDimensionalResizableImage::getChannels() const {
    return m_channels;
}

IEImageLocation IENDimensionalResizableImage::getLocation() const {
    return m_location;
}

void IENDimensionalResizableImage::setLocation(IEImageLocation t_location) {
    m_location = t_location;
}

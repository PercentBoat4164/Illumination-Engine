#pragma once

#include "stb_image.h"

#include <cassert>
#include <cstdint>
#include <mutex>
#include <vector>
#include <../contrib/stb/stb_image.h>

enum IEImageLocation {
    IE_IMAGE_LOCATION_NULL   = 0x0,
    IE_IMAGE_LOCATION_NONE   = 0x1,
    IE_IMAGE_LOCATION_SYSTEM = 0x2,
    IE_IMAGE_LOCATION_VIDEO  = 0x4
};


class aiTexture;

template<uint32_t width = 0, uint32_t height = 0, uint8_t channels = 0>
class IEImageNEW {
public:
    std::size_t                                  size;
    IEImageLocation                              location;
    std::array<char, width * height * channels> *data;

public:
    IEImageNEW() : size{width * height * channels}, location{IE_IMAGE_LOCATION_NONE}, data{} {
    }

    /** Copy a dynamically sized image into this statically sized image. */
    explicit IEImageNEW(IEImageNEW<0, 0, 0> &);

    /** Change the location(s) that the image is stored in.*/
    virtual void setLocation(IEImageLocation);

    /** Load assimp image data into the currently set location(s).*/
    virtual void uploadTexture(const aiTexture *){};

    /** Load stb_image data of size 'size' into the currently set location(s). */
    virtual void uploadTexture(stbi_uc *){};

    /** Load data into the currently set location(s). */
    virtual void uploadData(std::array<char, width * height * channels> *){};

    // Getters for the image attributes. The width, height, and channels must be virtual to allow the specialized
    // type to override them.

    [[nodiscard]] virtual uint32_t getWidth() const;

    [[nodiscard]] virtual uint32_t getHeight() const;

    [[nodiscard]] virtual uint8_t getChannels() const;

    [[nodiscard]] std::array<char, width * height * channels> *getData() const;

    [[nodiscard]] size_t getSize() const;

    [[nodiscard]] IEImageLocation getLocation() const;
};

template<>
class IEImageNEW<0, 0, 0> : public IEImageNEW<1, 1, 1> {
public:
    using IEImageNEW<1, 1, 1>::uploadTexture;
    using IEImageNEW<1, 1, 1>::setLocation;
    using IEImageNEW<1, 1, 1>::getSize;
    using IEImageNEW<1, 1, 1>::getLocation;

    virtual void setDimensions(uint32_t, uint32_t, uint8_t);

    virtual void uploadData(std::vector<char> *){};

    [[nodiscard]] uint32_t getWidth() const override;

    [[nodiscard]] uint32_t getHeight() const override;

    [[nodiscard]] uint8_t getChannels() const override;

    // Must not be made virtual. It must hide the previous getData().
    [[nodiscard]] std::vector<char> *getData() const;

private:
    uint32_t           width{};
    uint32_t           height{};
    uint8_t            channels{};
    std::vector<char> *data{};
};

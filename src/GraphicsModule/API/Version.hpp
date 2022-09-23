#pragma once

/* Include classes used as attributes or function arguments. */
// System dependencies
#include <cstdint>
#include <string>
#include <vector>

namespace IE::Graphics {
class Version {
public:
    std::string           name{"0.0.0"};
    std::vector<uint32_t> version{0, 0, 0};
    uint32_t              major{};
    uint32_t              minor{};
    uint32_t              patch{};
    uint32_t              number{};

    explicit Version(const std::string &versionName);

    explicit Version(const std::vector<uint32_t> &versionNumbers);

    Version(uint32_t versionMajor, uint32_t versionMinor, uint32_t versionPatch);

    explicit Version(uint32_t versionNumber);

    Version();
};
}  // namespace IE::Graphics
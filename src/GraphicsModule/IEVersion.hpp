#pragma once

/* Include classes used as attributes or function arguments. */
// System dependencies
#include <cstdint>
#include <string>
#include <vector>

struct IEVersion {
public:
    std::string           name{"0.0.0"};
    std::vector<uint32_t> version{0, 0, 0};
    uint32_t              major{};
    uint32_t              minor{};
    uint32_t              patch{};
    uint32_t              number{};

    explicit IEVersion(const std::string &versionName);

    explicit IEVersion(const std::vector<uint32_t> &versionNumbers);

    IEVersion(uint32_t versionMajor, uint32_t versionMinor, uint32_t versionPatch);

    explicit IEVersion(uint32_t versionNumber);

    IEVersion();
};
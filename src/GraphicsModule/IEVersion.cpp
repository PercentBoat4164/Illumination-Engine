/* Include this file's header. */
#include "IEVersion.hpp"

/* Include external dependencies. */
#include <vulkan/vulkan.h>


IEVersion::IEVersion(const std::string &versionName) {
    uint32_t nameLength = (name + ' ').find_first_of(' ');
    name = versionName.substr(0, nameLength);
    uint32_t firstBreak = name.find_first_of('.');
    uint32_t secondBreak = name.substr(firstBreak + 1, nameLength - firstBreak).find_first_of('.') + firstBreak + 1;
    major = std::stoul(name.substr(0, firstBreak));
    minor = std::stoul(name.substr(firstBreak + 1, secondBreak - firstBreak));
    patch = std::stoul(name.substr(secondBreak + 1, nameLength - secondBreak));
    version = {major, minor, patch};
    number = (major << 22) | (minor << 12) | patch;
    number = VK_MAKE_VERSION(major, minor, patch);
}

IEVersion::IEVersion(const std::vector<uint32_t> &versionNumbers) {
    version = versionNumbers;
    major = versionNumbers[0];
    minor = versionNumbers[1];
    patch = versionNumbers[2];
    name = std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
    number = VK_MAKE_VERSION(major, minor, patch);
}

IEVersion::IEVersion(uint32_t versionMajor, uint32_t versionMinor, uint32_t versionPatch) {
    major = versionMajor;
    minor = versionMinor;
    patch = versionPatch;
    name = std::to_string(versionMajor) + "." + std::to_string(versionMinor) + "." + std::to_string(versionPatch);
    version = {versionMajor, versionMinor, versionPatch};
    number = VK_MAKE_VERSION(major, minor, patch);
}

IEVersion::IEVersion(uint32_t versionNumber) {
    number = versionNumber;
    #ifdef ILLUMINATION_ENGINE_VULKAN
    major = VK_VERSION_MAJOR(versionNumber);
    minor = VK_VERSION_MINOR(versionNumber);
    patch = VK_VERSION_PATCH(versionNumber);
    #endif
    name = std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
    version = {major, minor, patch};
}

IEVersion::IEVersion() = default;

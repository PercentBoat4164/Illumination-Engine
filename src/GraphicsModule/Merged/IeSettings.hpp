#pragma once

struct IeVersion {
public:
    std::string name{"0.0.0"};
    std::vector<uint64_t> version{0, 0, 0};
    uint64_t major{};
    uint64_t minor{};
    uint64_t patch{};

    explicit IeVersion(const std::string& versionName) {
        uint64_t nameLength = (name + ' ').find_first_of(' ');
        name = versionName.substr(0, nameLength);
        uint64_t firstBreak = name.find_first_of('.');
        uint64_t secondBreak = name.substr(firstBreak + 1, nameLength - firstBreak).find_first_of('.') + firstBreak + 1;
        major = std::stoul(name.substr(0, firstBreak));
        minor = std::stoul(name.substr(firstBreak + 1, secondBreak - firstBreak));
        patch = std::stoul(name.substr(secondBreak + 1, nameLength - secondBreak));
        version = {major, minor, patch};
    }

    explicit IeVersion(const std::vector<uint64_t>& versionNumbers) {
        version = versionNumbers;
        major = versionNumbers[0];
        minor = versionNumbers[1];
        patch = versionNumbers[2];
        name = std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
    }

    IeVersion(uint64_t versionMajor, uint64_t versionMinor, uint64_t versionPatch) {
        major = versionMajor;
        minor = versionMinor;
        patch = versionPatch;
        name = std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
        version = {major, minor, patch};
    }

    IeVersion() = default;
};

struct IeSettings{
public:
    std::string applicationName{"Illumination Engine"};
    IeVersion applicationVersion{"0.0.0"};
    bool vSync{true};
    int32_t resolution[2] {800, 600};
    double renderResolutionScale {.5};
    int32_t renderResolution[2] = {static_cast<int32_t>(resolution[0] * renderResolutionScale), static_cast<int32_t>(resolution[1] * renderResolutionScale)};
    GLFWmonitor *monitor{};
    uint8_t msaaSamples{};
    uint32_t maxMipLevels{};
};
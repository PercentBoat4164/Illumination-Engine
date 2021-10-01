#pragma once

struct Version {
public:
    std::string name{"0.0.0"};
    std::vector<unsigned long> version{0, 0, 0};
    unsigned long major{};
    unsigned long minor{};
    unsigned long patch{};

    explicit Version(const std::string& versionName) {
        unsigned long nameLength = (name + ' ').find_first_of(' ');
        name = versionName.substr(0, nameLength);
        unsigned long firstBreak = name.find_first_of('.');
        unsigned long secondBreak = name.substr(firstBreak + 1, nameLength - firstBreak).find_first_of('.') + firstBreak + 1;
        major = std::stoul(name.substr(0, firstBreak));
        minor = std::stoul(name.substr(firstBreak + 1, secondBreak - firstBreak));
        patch = std::stoul(name.substr(secondBreak + 1, nameLength - secondBreak));
        version = {major, minor, patch};
    }

    explicit Version(const std::vector<unsigned long>& versionNumbers) {
        version = versionNumbers;
        major = versionNumbers[0];
        minor = versionNumbers[1];
        patch = versionNumbers[2];
        name = std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
    }

    Version(unsigned long versionMajor, unsigned long versionMinor, unsigned long versionPatch) {
        major = versionMajor;
        minor = versionMinor;
        patch = versionPatch;
        name = std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
        version = {major, minor, patch};
    }

    Version() = default;
};

struct Settings{
public:
    std::string applicationName{"Illumination Engine"};
    Version applicationVersion{"0.0.0"};
    bool vSync{true};
    int resolution[2] {800, 600};
    GLFWmonitor *monitor{};
};
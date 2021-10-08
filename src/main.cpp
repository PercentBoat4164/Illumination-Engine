#include "GraphicsModule/Merged/IeRenderEngine.hpp"

#include <sstream>
#include <iostream>
#include <cstring>

template <typename ObjectType> void saveObjectToFile(ObjectType *objectToSave, const std::string& filename) {
    unsigned long objectSize = sizeof(ObjectType);
    std::ofstream file{};
    std::vector<char> fileContents{static_cast<char>(objectSize)};
    std::memcpy(fileContents.data(), reinterpret_cast<void *>(objectToSave), objectSize);
    file.open(filename);
    file.write(fileContents.data(), static_cast<std::streamsize>(objectSize));
    std::flush(file);
    file.close();
}

template <typename ObjectType> ObjectType loadObjectFromFile(ObjectType *objectToLoad, const std::string& filename) {
    unsigned long objectSize = sizeof(ObjectType);
    std::ifstream file{};
    file.open(filename);
    std::stringstream fileContents{};
    fileContents << file.rdbuf();
    file.close();
    ObjectType result{"OpenGL"};
    std::memcpy(reinterpret_cast<void *>(&result), reinterpret_cast<const void *>(fileContents.str().c_str()), objectSize);
    return result;
}

int main(int argc, char **argv) {
    std::string selection;
    std::string input;
    if (argc > 1) {
        if (*argv[1] == 'v') { input = "Vulkan"; }
        if (*argv[1] == 'o') { input = "OpenGL"; }
    } else {
        std::cout << "'v': Run Vulkan render engine\n'o': Run OpenGL render engine\n";
        std::cin >> selection;
        if (selection[0] == 'v') { input = "Vulkan"; }
        if (selection[0] == 'o') { input = "OpenGL"; }
    }
    Log log{};
    log.create("Illumination Engine");
    IeRenderEngine renderEngine{input, &log};
    renderEngine.create();
}
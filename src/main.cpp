#include "GraphicsModule/IERenderEngine.cpp"
#include "InputModule/IEKeyboard.hpp"
#include "Core/AssetModule/IEAsset.hpp"
#include "Core/FileSystemModule/IEFileSystem.hpp"

int main() {
    IERenderEngine renderEngine{};
    IERenderable cube{&renderEngine, "res/Models/Cube/cube.obj"};
    IEKeyboard keyboard{renderEngine.window};
    IEWindowUser windowUser{&renderEngine, &keyboard};
    glfwSetWindowUserPointer(renderEngine.window, &windowUser);
    IEAsset asset{};
    asset.addAspect(&cube);
    renderEngine.addAsset(&asset);
    asset.scale = {1.0, 1.0, 1.0};
    cube.scale = {1.0, 1.0, 1.0};
    while (renderEngine.update()) {
        glfwPollEvents();
        keyboard.handleQueue();
    }
}
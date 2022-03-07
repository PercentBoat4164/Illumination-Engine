#include "GraphicsModule/IERenderEngine.cpp"
#include "InputModule/IEKeyboard.hpp"
#include "Core/AssetModule/IEAsset.hpp"

int main() {
    IERenderEngine renderEngine{};
    IERenderable cube{&renderEngine, "res/Models/Cube/cube.obj"};
    IEKeyboard keyboard{renderEngine.window};
    IEWindowUser windowUser{&renderEngine, &keyboard};
    glfwSetWindowUserPointer(renderEngine.window, &windowUser);
    renderEngine.loadRenderable(&cube);
    IEAsset asset{};
    asset.addAspect(&cube);
}
#include "Core/FileSystemModule/IEFileSystem.hpp"
#include "Core/MultiDimensionalVector.hpp"
#include "Core/ThreadingModule/ThreadPool.hpp"
#include "GraphicsModule/IERenderEngine.hpp"
#include "InputModule/IEKeyboard.hpp"

int main() {
    IE::Core::MultiDimensionalVector<unsigned char> image{10u};
    image.resize(13u);                    // Required
    auto subImage = image[{{-1}, {10}}];  // Required
    std::cout << subImage << std::endl;   // Would be cool
}
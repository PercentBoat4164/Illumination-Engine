#include <filesystem>
#include <iostream>

std::filesystem::path getExecutablePath()
{
    return std::filesystem::canonical("/proc/self/exe");
}

int main() {
    std::cout << getExecutablePath() << std::endl;
    return 0;
}
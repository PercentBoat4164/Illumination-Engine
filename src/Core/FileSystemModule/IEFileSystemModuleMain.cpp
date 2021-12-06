#include "IEFileSystem.hpp"

int main(int argc, char **argv) {
    IEFile file{};
    file.path = "/path/to/file.txt";
    file.getExtensions();
    file.getName();
    return 0;
}
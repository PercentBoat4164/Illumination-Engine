#include <vector>
#include <string>

class IEFile {
public:
    std::string path;
    std::string name;
    std::vector<std::string> extensions;
    std::string contents;

    std::vector<std::string> getExtensions() {
        std::string originalPath;
        std::swap(originalPath, path);
        extensions = std::vector<std::string>{};
        uint32_t lastExtensionPosition{static_cast<uint32_t>(path.find_last_of('.'))};
        while (lastExtensionPosition < path.size()) {
            extensions.push_back(path.substr(lastExtensionPosition, path.size() - lastExtensionPosition));
            path = path.substr(0, lastExtensionPosition);
            lastExtensionPosition = path.find_last_of('.');
        }
        std::swap(path, originalPath);
        return extensions;
    }

    std::string getName() {
        uint32_t nameStartPosition{static_cast<uint32_t>(path.find_last_of('/') + 1)}; // starting position of name in path
        name = path.substr(nameStartPosition, path.size() - nameStartPosition);
        return name;
    }
};
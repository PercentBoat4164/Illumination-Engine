#include "Script.hpp"

#include "PythonScript.hpp"

#include <functional>

std::unordered_map<std::string, std::function<std::unique_ptr<IE::Script::Script>(const std::string &)>>
  IE::Script::Script::extensionsToLanguage{
    {".py", &IE::Script::detail::PythonScript::create}
};

std::unique_ptr<IE::Script::Script> IE::Script::Script::create(const std::string &filename) {
    std::string extension = filename.substr(filename.find_last_of('.'));
    return extensionsToLanguage.at(extension)(filename);
}

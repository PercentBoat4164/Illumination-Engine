#include "Script.hpp"

#include "PythonScript.hpp"

#include <functional>

std::unordered_map<std::string, std::function<std::shared_ptr<IE::Script::Script>(IE::Core::File *)>>
  IE::Script::Script::extensionsToLanguage{
    {".py", &IE::Script::detail::PythonScript::create}
};

std::shared_ptr<IE::Script::Script> IE::Script::Script::create(IE::Core::File *t_file) {
    return extensionsToLanguage.at(t_file->extension)(t_file);
}

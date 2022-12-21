#include "ScriptEngine.hpp"

#include "JavaScript.hpp"
#include "PythonScript.hpp"

const std::unordered_map<std::string, std::function<std::shared_ptr<IE::Script::Script>(std::shared_ptr<IE::Core::Engine>, IE::Core::File *)>>IE::Script::ScriptEngine::extensionsToLanguage{
    {".py",  &IE::Script::detail::PythonScript::create},
    {".jar", &IE::Script::detail::JavaScript::create  }
};
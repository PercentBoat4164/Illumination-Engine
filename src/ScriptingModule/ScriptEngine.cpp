#include "ScriptEngine.hpp"

#include "Core/FileSystemModule/File.hpp"
#include "JavaScript.hpp"
#include "PythonScript.hpp"

const std::unordered_map<
  std::string,
  std::function<std::shared_ptr<IE::Script::ScriptEngine::AspectType>(IE::Core::Engine *, IE::Core::File *)>>
  IE::Script::ScriptEngine::extensionsToLanguage{
    {".py",  &IE::Script::detail::PythonScript::create},
    {".jar", &IE::Script::detail::JavaScript::create  },
};

std::shared_ptr<IE::Script::Script>
IE::Script::ScriptEngine::createAspect(const std::string &t_id, IE::Core::File *t_resource) {
    std::shared_ptr<AspectType> aspect = _getAspect<AspectType>(t_id);
    if (aspect == nullptr) {
        aspect = extensionsToLanguage.at(t_resource->extension)(this, t_resource);
        std::unique_lock<std::mutex> lock(m_aspectsMutex);
        m_aspects[t_id] = aspect;
    }
    return aspect;
}

std::shared_ptr<IE::Core::Engine> IE::Script::ScriptEngine::create() {
    return std::shared_ptr<IE::Script::ScriptEngine>();
}

bool IE::Script::ScriptEngine::update() {
    return false;
}

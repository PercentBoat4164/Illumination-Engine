/* Include this file's header*/
#include "IEScript.hpp"

/* Include dependencies from Core. */
#include "Core/FileSystemModule/IEFile.hpp"

IEScript::IEScript(IEFile *initialFile) {
    file = initialFile;
}

std::string IEScript::deduceLanguage() {
    language = std::string{};
    std::string thisExtension = file->path.substr(file->path.find_last_of('.'));
    for (uint8_t i = 0; i < languageFileExtensions.size() && language.empty(); ++i) {
        for (uint8_t j = 0; j < languageFileExtensions[i].size() && language.empty(); ++j) {
            if (languageFileExtensions[i][j] == thisExtension) {
                language = supportedLanguages[i];
            }
        }
    }
    return language;
}

void IEScript::compileInPlace() {
    try {
        luaContext.AddGlobalVariable("i", std::make_shared<LuaCpp::Engine::LuaTNumber>(0));
        luaContext.CompileString("update", file->read(file->length, 0));
    }
    catch (std::runtime_error& e) {
        printf("%s", e.what());
    }
}

void IEScript::execute() {
    luaContext.Run("update");
}

#pragma once

#include "Core/AssetModule/IEAspect.hpp"
#include "Core/FileSystemModule/IEFile.hpp"
#include "LuaCpp.hpp"

class IEScript : public IEAspect {
private:
    std::vector<std::string> supportedLanguages{"C++", "Java", "Python", "Lua", "C#"};
    std::vector<std::vector<std::string>> languageFileExtensions{
        {".h", ".c", ".hpp", ".cpp", ".cxx"},  // recognized C/C++ file extension(s)
        {".java"},  // recognized Java file extension(s)
        {".py"},  // recognized Python file extension(s)
        {".lua"},  // recognized Lua file extension(s)
        {".cs"},  // recognized C# file extension(s)
    };

public:
    IEFile *file{};
    std::string language{};
    LuaCpp::LuaContext luaContext{};

    IEScript() = default;

    explicit IEScript(IEFile *initialFile) {
        file = initialFile;
    }

    std::string deduceLanguage() {
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

    /**@todo Add code to compile / pre-process / prepare the program files. The result of such a piece of code should be pointers to executable functions.*/
    void compileInPlace() {
        try {
            luaContext.CompileString("update", file->read(file->length, 0));
        }
        catch (std::runtime_error& e) {
            printf("%s", e.what());
        }
    }

    void execute() {
        luaContext.Run("update");
    }
};
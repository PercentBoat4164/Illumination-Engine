#pragma once

/* Predefine classes used with pointers or as return values for functions. */
class IEFile;

/* Include classes used as attributes or function arguments. */
// Modular dependencies
#include "Core/AssetModule/IEAspect.hpp"

// External dependencies
#include <LuaCpp.hpp>

// System dependencies
#include <string>


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

    explicit IEScript(IEFile *initialFile);

    std::string deduceLanguage();

    /**@todo Add code to compile / pre-process / prepare the program files. The result of such a piece of code should be pointers to executable functions.*/
    void compileInPlace();

    void execute();
};
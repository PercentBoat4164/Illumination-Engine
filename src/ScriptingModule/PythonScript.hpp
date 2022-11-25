#pragma once

#include "Core/FileSystemModule/File.hpp"
#include "ScriptingModule/Script.hpp"

#include <Python.h>
#include <utility>

namespace IE::Script::detail {
class PythonScript : public IE::Script::Script {
private:
    static uint32_t m_pythonScripts;

    IE::Core::File *m_file;

    //    PyInterpreterState *state;

public:
    PythonScript(Core::File *t_file);

    void update() override;

    void initialize() override;

    void compile() override;

    static std::shared_ptr<Script> create(IE::Core::File *t_file) {
        return std::make_shared<IE::Script::detail::PythonScript>(t_file);
    }
};
}  // namespace IE::Script::detail
#pragma once

#include "ScriptingModule/Script.hpp"

#include <Python.h>

namespace IE::Script::detail {
class PythonScript : public IE::Script::Script {
private:
    std::string     m_scriptName;
    IE::Core::File *m_file;
    PyObject       *scriptUpdate;
    PyObject       *scriptInitialize;

    //    PyInterpreterState *state;
public:
    PythonScript(
      IE::Core::Engine *t_engine,
      IE::Core::File   *t_file
    );

    void update() override;

    void initialize() override;

    void load() override;

    void compile() override;

    static std::shared_ptr<Script> create(IE::Core::Engine *t_engine, IE::Core::File *t_file);
};
}  // namespace IE::Script::detail
#pragma once

#include "ScriptingModule/Script.hpp"

#include <Python.h>
#include <utility>

namespace IE::Script::detail {
class PythonScript : public IE::Script::Script {
private:
    static uint32_t m_pythonScripts;

    std::string m_filename;

    //    PyInterpreterState *state;

public:
    explicit PythonScript(std::string t_filename);

    void update() override;

    void initialize() override;

    void compile() override;

    static std::unique_ptr<Script> create(const std::string &t_filename) {
        return std::make_unique<IE::Script::detail::PythonScript>(t_filename);
    }
};
}  // namespace IE::Script::detail
#include "PythonScript.hpp"

#define ILLUMINATION_ENGINE_INTERNAL_PYTHON_SETUP_SCRIPT "res/scripts/__INTERNAL__setup.py"

uint32_t IE::Script::detail::PythonScript::m_pythonScripts{};

void IE::Script::detail::PythonScript::update() {
    PyRun_SimpleString("script.update()");
}

void IE::Script::detail::PythonScript::initialize() {
    PyRun_SimpleString(("from importlib.machinery import SourceFileLoader\nscript = SourceFileLoader(\"" +
                        m_file->name + "\",\"" + m_file->path.string() + "\").load_module()")
                         .c_str());
    PyRun_SimpleString("script.initialize()");
}

void IE::Script::detail::PythonScript::compile() {
    std::filesystem::path path(ILLUMINATION_ENGINE_INTERNAL_PYTHON_SETUP_SCRIPT);
    PyRun_SimpleString(("from importlib.machinery import SourceFileLoader\ncython_compiler = SourceFileLoader(\"" +
                        path.filename().string() + "\",\"" + path.string() +
                        "\").load_module()\ncython_compiler.cython_compile('" + m_file->path.string() + "')")
                         .c_str());
    PyRun_SimpleString(("cython_compiler.cython_compile('" + m_file->path.string() + "')").c_str());
    path = "build/lib.linux-x86_64-3.10/rotate.cpython-310-x86_64-linux-gnu.so";
}

IE::Script::detail::PythonScript::PythonScript(IE::Core::File *t_file) : m_file(t_file) {
    if (Py_IsInitialized() == 0 && m_pythonScripts == 0) {
        Py_Initialize();
    } else if (Py_IsInitialized() == 0 && m_pythonScripts > 0 || Py_IsInitialized() > 0 && m_pythonScripts == 0) {
        //            logError();
    }
    ++m_pythonScripts;
}

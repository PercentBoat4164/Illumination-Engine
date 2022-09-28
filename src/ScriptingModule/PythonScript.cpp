#include "PythonScript.hpp"

#define ILLUMINATION_ENGINE_INTERNAL_PYTHON_SETUP_SCRIPT "res/scripts/__INTERNAL__setup.py"

uint32_t IE::Script::detail::PythonScript::m_pythonScripts{};

void IE::Script::detail::PythonScript::update() {
    PyRun_SimpleString("rotate.update()");
}

void IE::Script::detail::PythonScript::initialize() {
    PyRun_SimpleString("rotate.initialize()");
}

void IE::Script::detail::PythonScript::compile() {
    /// COMPILE PYTHON TO SHARED OBJECT
    auto *file{fopen(ILLUMINATION_ENGINE_INTERNAL_PYTHON_SETUP_SCRIPT, "re")};
    if (file == nullptr) {
        // log error
    }
    size_t   size{sizeof(ILLUMINATION_ENGINE_INTERNAL_PYTHON_SETUP_SCRIPT)};
    wchar_t *argv0{Py_DecodeLocale(ILLUMINATION_ENGINE_INTERNAL_PYTHON_SETUP_SCRIPT, &size)};
    size = m_filename.size();
    wchar_t                 *argv1{Py_DecodeLocale(m_filename.c_str(), &size)};
    std::array<wchar_t *, 2> argv{argv0, argv1};
    PySys_SetArgv(2, argv.data());
    PyRun_AnyFile(file, ILLUMINATION_ENGINE_INTERNAL_PYTHON_SETUP_SCRIPT);
    PySys_SetArgv(0, nullptr);
    if (fclose(file) != 0) {
        // log error using errno
    }
    std::string importPath{
      "/home/thaddeus/Documents/Programming/C++/Illumination-Engine/cmake-build-debug-gcc-v12/bin/build/"
      "lib.linux-x86_64-3.10"};
    size = importPath.size();
    PySys_SetPath(Py_DecodeLocale(importPath.c_str(), &size));
    PyRun_SimpleString("import rotate");
}

IE::Script::detail::PythonScript::PythonScript(std::string t_filename) : m_filename(std::move(t_filename)) {
    //        PyInterpreterState_New();
    if (Py_IsInitialized() == 0 && m_pythonScripts == 0) {
        Py_Initialize();
        ++m_pythonScripts;
    } else if (Py_IsInitialized() == 0 && m_pythonScripts > 0 || Py_IsInitialized() > 0 && m_pythonScripts == 0) {
        //            logError();
    }
}

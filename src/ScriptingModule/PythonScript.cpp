#include "PythonScript.hpp"

#include "Core/Core.hpp"

#define ILLUMINATION_ENGINE_INTERNAL_PYTHON_SETUP_SCRIPT std::string("res.scripts.setup")

void IE::Script::detail::PythonScript::update() {
    PyObject_CallNoArgs(scriptUpdate);
}

void IE::Script::detail::PythonScript::initialize() {
    PyObject_CallNoArgs(scriptInitialize);
}

void IE::Script::detail::PythonScript::load() {
    m_scriptName     = m_file->name.substr(0, m_file->name.find_last_of('.'));  // Get rid of .py
    PyObject *module = PyImport_Import(PyUnicode_FromString(m_scriptName.c_str()));
    scriptInitialize = PyObject_GetAttrString(module, "initialize");
    scriptUpdate     = PyObject_GetAttrString(module, "update");
}

void IE::Script::detail::PythonScript::compile() {
    std::filesystem::path path(ILLUMINATION_ENGINE_INTERNAL_PYTHON_SETUP_SCRIPT);
    PyObject             *compiler = PyObject_GetAttrString(
      PyImport_Import(PyUnicode_FromString(ILLUMINATION_ENGINE_INTERNAL_PYTHON_SETUP_SCRIPT.c_str())),
      "cython_compile"
    );
    PyObject_CallOneArg(compiler, PyUnicode_FromString(m_file->path.c_str()));
}

IE::Script::detail::PythonScript::PythonScript(IE::Core::Engine *t_engine, IE::Core::File *t_file) :
        Script(t_engine, t_file),
        m_file(t_file) {
    if (!Py_IsInitialized()) {
        Py_Initialize();
        std::string path = IE::Core::Core::getFileSystem()->getBaseDirectory();
        wchar_t    *wc   = reinterpret_cast<wchar_t *>(malloc(path.size() * sizeof(wchar_t)));
        mbstowcs(wc, path.c_str(), path.size());
        PySys_SetArgv(0, &wc);
        delete wc;
    }
}

std::shared_ptr<IE::Script::Script>
IE::Script::detail::PythonScript::create(IE::Core::Engine *t_engine, IE::Core::File *t_file) {
    return std::make_shared<IE::Script::detail::PythonScript>(t_engine, t_file);
}

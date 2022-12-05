#include "PythonScript.hpp"

#define ILLUMINATION_ENGINE_INTERNAL_PYTHON_SETUP_SCRIPT std::string("res.scripts.setup")

uint32_t IE::Script::detail::PythonScript::m_pythonScripts{};

void IE::Script::detail::PythonScript::update() {
    PyObject_CallNoArgs(scriptUpdate);
}

void IE::Script::detail::PythonScript::initialize() {
    PyObject_CallNoArgs(scriptInitialize);
}

void IE::Script::detail::PythonScript::load() {
    m_scriptName     = m_file->name.substr(0, m_file->name.find_last_of('.'));
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

IE::Script::detail::PythonScript::PythonScript(IE::Core::File *t_file) : m_file(t_file) {
    if (Py_IsInitialized() == 0 && m_pythonScripts == 0) {
        Py_Initialize();
        std::string path = IE::Core::Core::getFileSystem()->getBaseDirectory();
        wchar_t    *wc   = reinterpret_cast<wchar_t *>(malloc(path.size() * sizeof(wchar_t)));
        mbstowcs(wc, path.c_str(), path.size());
        PySys_SetArgv(0, &wc);
        delete wc;
    } else if (Py_IsInitialized() == 0 && m_pythonScripts > 0 || Py_IsInitialized() > 0 && m_pythonScripts == 0) {
    }
    ++m_pythonScripts;
}

std::shared_ptr<IE::Script::Script> IE::Script::detail::PythonScript::create(IE::Core::File *t_file) {
    return std::make_shared<IE::Script::detail::PythonScript>(t_file);
}

set(IEScriptingModuleSourceFiles  # Gather sources
		Script.cpp
		PythonScript.cpp)

# Create and define properties for the library
add_library(IEScriptingModule ${IEScriptingModuleSourceFiles})
set_target_properties(IEScriptingModule PROPERTIES LINKER_LANGUAGE CXX)

# Python for scripting language support
execute_process(COMMAND python3 ${ILLUMINATION_ENGINE_BASE_DIRECTORY}/venv/bin/activate_this.py && pip install cython && pip install setuptools)
set(Python_FIND_VIRTUALENV ONLY)
find_package(Python COMPONENTS Development)

target_link_libraries(IEScriptingModule PUBLIC IECore Python::Python)
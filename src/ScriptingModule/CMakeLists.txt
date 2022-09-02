set(IEScriptingModuleSourceFiles  # Gather sources
		IEScript.hpp
		IEScript.cpp
		)

# Create and define properties for the library
add_library(IEScriptingModule ${IEScriptingModuleSourceFiles})
set_target_properties(IEScriptingModule PROPERTIES LINKER_LANGUAGE CXX)

# Add external dependency libraries and include directories to the target
target_link_libraries(IEScriptingModule PUBLIC swig)

# Compiles without this line, but without it CLion thinks it cannot find the files.
target_include_directories(IEScriptingModule PUBLIC ${swig_DIR})

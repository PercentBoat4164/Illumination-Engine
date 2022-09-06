set(IEScriptingModuleSourceFiles  # Gather sources
		IEScript.hpp
		IEScript.cpp
		)

# Create and define properties for the library
add_library(IEScriptingModule ${IEScriptingModuleSourceFiles})
set_target_properties(IEScriptingModule PROPERTIES LINKER_LANGUAGE CXX)

target_link_libraries(IEScriptingModule PUBLIC IECore sol2 ${LUA_LIBRARIES})
target_include_directories(IEScriptingModule PUBLIC ${LUA_INCLUDE_DIR} ${sol2_DIR})
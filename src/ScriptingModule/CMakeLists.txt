set(IEScriptingModuleSourceFiles  # Gather sources
        IEScript.cpp
        IEScript.hpp
        )

# Create and define properties for the library
add_library(IEScriptingModule ${IEScriptingModuleSourceFiles})
target_link_libraries(IEScriptingModule PUBLIC IECore)
set_target_properties(IEScriptingModule PROPERTIES LINKER_LANGUAGE CXX)

# Add external dependency libraries and include directories to the target
if (Lua_FOUND)
    target_link_libraries(IEScriptingModule PUBLIC luacpp)
    target_include_directories(IEScriptingModule PUBLIC ${luacpp_DIR})
endif ()
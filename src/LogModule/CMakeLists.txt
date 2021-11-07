function (addLog4cplus target)
    if ((NOT EXISTS ${CMAKE_BINARY_DIR}/Log4cplusFOUND) AND EXISTS ${CMAKE_SOURCE_DIR}/deps/log4cplus)
        add_subdirectory(${CMAKE_SOURCE_DIR}/deps/log4cplus ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/log4cplus)
    endif ()
    target_link_libraries(${target} PUBLIC log4cplus)
    file(TOUCH ${CMAKE_BINARY_DIR}/Log4cplusFOUND)
    add_compile_definitions(ILLUMINATION_ENGINE_LOG)
    if (NOT CMAKE_BUILD_TYPE STREQUAL Debug)
        add_compile_definitions(LOG4CPLUS_DISABLE_DEBUG)
    endif ()
endfunction ()

set(IELogModuleSourceFiles
        Log.hpp
        IELogModuleMain.cpp)
add_executable(IELogModule ${IELogModuleSourceFiles})
set_target_properties(IELogModule PROPERTIES LINKER_LANGUAGE CXX)
addLog4cplus(IELogModule)
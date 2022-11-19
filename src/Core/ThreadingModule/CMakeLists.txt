set(IEThreadingModuleSourceFiles  # Gather sources
        JobQueue.cpp
        ThreadPool.cpp
        WorkerThread.cpp
        )


add_library(IEThreadingModule ${IEThreadingModuleSourceFiles})
target_link_libraries(IEThreadingModule PUBLIC INT_src)
set_target_properties(IELogModule PROPERTIES LINKER_LANGUAGE CXX)
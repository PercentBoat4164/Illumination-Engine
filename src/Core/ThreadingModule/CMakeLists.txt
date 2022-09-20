set(IEThreadingModuleSourceFiles  # Gather sources
        JobQueue.cpp
        ThreadPool.cpp
        WorkerThread.cpp
        )


add_library(IEThreadingModule ${IEThreadingModuleSourceFiles})
set_target_properties(IELogModule PROPERTIES LINKER_LANGUAGE CXX)
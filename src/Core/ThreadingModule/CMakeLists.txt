set(IEThreadingModuleSourceFiles  # Gather sources
        ThreadPool/JobQueue.cpp
        ThreadPool/ThreadPool.cpp
        ThreadPool/WorkerThread.cpp
        )


add_library(IEThreadingModule ${IEThreadingModuleSourceFiles})
set_target_properties(IELogModule PROPERTIES LINKER_LANGUAGE CXX)
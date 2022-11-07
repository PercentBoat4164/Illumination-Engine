set(IEThreadingModuleSourceFiles  # Gather sources
		ThreadPool/JobQueue.cpp
		ThreadPool/ThreadPool.cpp
		ThreadPool/WorkerThread.cpp
		)


add_library(IEThreadingModule ${IEThreadingModuleSourceFiles})
set_target_properties(IEThreadingModule PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(IEThreadingModule PUBLIC INT_src)
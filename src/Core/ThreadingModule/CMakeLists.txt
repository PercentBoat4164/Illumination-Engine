set(IEThreadingModuleSourceFiles  # Gather sources
		CoroutineTask.cpp
		FunctionTask.cpp
		Pool.cpp
		Queue.cpp
		Task.cpp
		ThreadPool.cpp
		)


add_library(IEThreadingModule ${IEThreadingModuleSourceFiles})
set_target_properties(IEThreadingModule PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(IEThreadingModule PUBLIC INT_src)
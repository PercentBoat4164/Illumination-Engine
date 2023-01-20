set(IEThreadingModuleSourceFiles  # Gather sources
		CoroutineTask.cpp
		FunctionTask.cpp
		Pool.cpp
		Queue.cpp
		Task.cpp
		ThreadPool.cpp
		)


add_library(IEThreadingModule ${IEThreadingModuleSourceFiles})
target_link_libraries(IEThreadingModule PUBLIC INT_src)
set_target_properties(IELogModule PROPERTIES LINKER_LANGUAGE CXX)
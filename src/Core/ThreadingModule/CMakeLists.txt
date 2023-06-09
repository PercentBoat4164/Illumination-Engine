set(IEThreadingModuleSourceFiles  # Gather sources
  Awaitable.cpp
  BaseTask.cpp
  EnsureThread.cpp
  Queue.cpp
  ResumeAfter.cpp
  Task.cpp
  ThreadPool.cpp
  Worker.cpp
  )


add_library(IEThreadingModule ${IEThreadingModuleSourceFiles})
target_link_libraries(IEThreadingModule PUBLIC INT_src)
set_target_properties(IEThreadingModule PROPERTIES LINKER_LANGUAGE CXX)

#pragma once

#include "BaseTask.hpp"

namespace IE::Core::Threading {
class ThreadPool;

class Worker {
public:
    static void start(ThreadPool *t_threadPool);

    static void waitForTask(ThreadPool *t_threadPool, BaseTask &t_task);
};
}  // namespace IE::Core::Threading

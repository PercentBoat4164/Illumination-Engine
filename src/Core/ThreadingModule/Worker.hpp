#pragma once

namespace IE::Core::Threading {
class ThreadPool;
class BaseTask;

class Worker {
public:
    static void start(ThreadPool *t_threadPool);

    static void loopUntilTaskFinished(ThreadPool *t_threadPool, BaseTask *t_stopTask);
};
}  // namespace IE::Core::Threading

#pragma once

namespace IE::Core::Threading {
class ThreadPool;

class Worker {
public:
    static void start(ThreadPool *t_threadPool);
};
}  // namespace IE::Core::Threading

#pragma once

#include <mutex>
#include <queue>

// Thread safe implementation of a Queue using an std::queue
template<typename T>
class JobQueue {
private:
    std::queue<T> m_queue;
    std::mutex    m_mutex;

public:
    bool empty();

    int size();

    void enqueue(T &t);

    bool dequeue(T &t);
};
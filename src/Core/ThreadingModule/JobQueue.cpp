#include "JobQueue.hpp"

template<typename T>
bool JobQueue<T>::empty() {
    std::unique_lock<std::mutex> lock(m_mutex);
    return m_queue.empty();
}

template<typename T>
int JobQueue<T>::size() {
    std::unique_lock<std::mutex> lock(m_mutex);
    return m_queue.size();
}

template<typename T>
void JobQueue<T>::enqueue(T &t) {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_queue.push(t);
}

template<typename T>
bool JobQueue<T>::dequeue(T &t) {
    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_queue.empty()) return false;
    t = m_queue.front();
    m_queue.pop();
    return true;
}

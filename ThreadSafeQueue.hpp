#ifndef _THREAD_SAFE_QUEUE_HPP
#define _THREAD_SAFE_QUEUE_HPP

#include <mutex>
#include <queue>

template <typename T>
class ThreadSafeQueue {
  public:
    void push(T value);
    T pop();

    unsigned int count();
    bool empty();

  private:
    std::mutex lock;
    std::queue<T> queue;
};

template <typename T>
unsigned int ThreadSafeQueue<T>::count() {
    std::lock_guard<std::mutex> lg(lock);
    return queue.size();
}

template <typename T>
bool ThreadSafeQueue<T>::empty() {
    std::lock_guard<std::mutex> lg(lock);
    return queue.empty();
}

template <typename T>
void ThreadSafeQueue<T>::push(T value) {
    std::lock_guard<std::mutex> lg(lock);
    queue.push(value);
}

template <typename T>
T ThreadSafeQueue<T>::pop() {
    std::lock_guard<std::mutex> lg(lock);
    T value(std::move(queue.front()));
    queue.pop();
    return value;
}

#endif


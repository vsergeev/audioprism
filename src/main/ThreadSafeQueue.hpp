#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <chrono>

template <typename T>
class ThreadSafeQueue {
  public:
    void push(T value);
    T pop();

    size_t count();
    bool empty();

    template <typename Rep, typename Period>
    bool wait(const std::chrono::duration<Rep, Period> &rel_time);

  private:
    std::mutex _lock;
    std::condition_variable _cvNotEmpty;
    std::queue<T> _queue;
};

template <typename T>
size_t ThreadSafeQueue<T>::count() {
    std::lock_guard<std::mutex> lg(_lock);
    return _queue.size();
}

template <typename T>
bool ThreadSafeQueue<T>::empty() {
    std::lock_guard<std::mutex> lg(_lock);
    return _queue.empty();
}

template <typename T>
void ThreadSafeQueue<T>::push(T value) {
    std::lock_guard<std::mutex> lg(_lock);
    _queue.push(value);
    _cvNotEmpty.notify_one();
}

template <typename T>
T ThreadSafeQueue<T>::pop() {
    std::unique_lock<std::mutex> lg(_lock);
    while (_queue.empty())
        _cvNotEmpty.wait(lg);
    T value(std::move(_queue.front()));
    _queue.pop();
    return value;
}

template <typename T>
template <typename Rep, typename Period>
bool ThreadSafeQueue<T>::wait(const std::chrono::duration<Rep, Period> &rel_time) {
    std::unique_lock<std::mutex> lg(_lock);
    while (_queue.empty())
        return (_cvNotEmpty.wait_for(lg, rel_time) == std::cv_status::no_timeout);
    return true;
}

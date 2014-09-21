#ifndef _THREADSAFERESOURCE_HPP
#define _THREADSAFERESOURCE_HPP

#include <mutex>

template <typename T>
class ThreadSafeResource {
  public:
    ThreadSafeResource(T &resource) : resource(resource) { }

    ThreadSafeResource(const ThreadSafeResource &rv) = delete;
    ThreadSafeResource(ThreadSafeResource &&rv) = delete;
    ThreadSafeResource &operator=(const ThreadSafeResource &rv) = delete;
    ThreadSafeResource &operator=(ThreadSafeResource &&rv) = delete;

    T& get() { return resource; }
    void lock() { _lock.lock(); }
    void unlock() { _lock.unlock(); }

  private:
    T &resource;
    std::mutex _lock;
};

#endif


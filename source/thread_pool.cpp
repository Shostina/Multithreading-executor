#include "thread_pool.h"

thread_local ThreadPool *ins;

void ThreadPool::thread_fn(std::atomic_bool * enabled) {
    ins = this;
    while (*enabled) {
        std::unique_lock<std::mutex> locker(mutex);
        cv.wait(locker, [&]() { return !tasks.empty() || !(*enabled); });
        if (!tasks.empty()) {
            std::function<void()> task;
            task = tasks.front();
            tasks.pop();
            locker.unlock();
            task();
        }
    }
}

ThreadPool::ThreadPool(size_t threads_s) {
    if (threads_s <= 0) {
        throw std::logic_error("Expected positive integer number of threads");
    }
    *enabled = true;
    for (size_t i = 0; i < threads_s; i++) {
        threads.emplace_back(&ThreadPool::thread_fn, this, enabled);
    }
}

ThreadPool::~ThreadPool() {
    (*enabled) = false;
    cv.notify_all();
    for (auto &thread_fn : threads) {
        if (thread_fn.joinable()) {
            thread_fn.join();
        }
    }
}

void ThreadPool::execute(std::function<void()> task) {
    std::unique_lock<std::mutex> locker(mutex);
    tasks.push(task);
    cv.notify_all();
}

#ifndef _THREADPOOL_H
#define _THREADPOOL_H

#include <stdexcept>
#include <functional>
#include <thread>
#include <queue>
#include <mutex>
#include <memory>
#include <condition_variable>
#include <atomic>

class ThreadPool;

extern thread_local ThreadPool *ins;

//typedef std::function<void()> task_type;

class ThreadPool {

private:
    std::atomic_bool *enabled;
    std::mutex mutex;
    std::condition_variable cv;
    std::queue<std::function<void()>> tasks;
    std::vector<std::thread> threads;

    void thread_fn(std::atomic_bool *enabled);

public:

    ThreadPool(size_t threads_s);

    ~ThreadPool();

    void execute(std::function<void()> task);

    template<typename T, typename F>
    void parallel(T begin, T end, F func) {
        for(T i = begin; i != end; i++) {;
            execute([&func, i]() {
                func(*i);
            });
        }
        std::atomic_bool p;
        p = true;
        execute([&p]() {
            (p) = false;
        });
        thread_fn(&p);
    }
};

#endif

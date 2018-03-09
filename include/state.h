#ifndef _STATE_H
#define _STATE_H
#include <memory>
#include <mutex>
#include <vector>
#include <list>
#include <deque>
#include <set>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <numeric>
#include <exception>
#include <condition_variable>
#include <atomic>



template<typename T>
struct State {
    bool isValueGot;
    bool isExceptionGot;
    std::exception_ptr exception;
    T value;
    std::mutex mut;
    std::condition_variable cv;

    State() : isValueGot(false), isExceptionGot(false) { }
    ~State() = default;
};

template <>
struct State<void> {
    bool isValueGot;
    bool isExceptionGot;
    std::exception_ptr exception;
    std::mutex mut;
    std::condition_variable cv;
    State() : isValueGot(false), isExceptionGot(false) { }
    ~State() = default;

};

template <typename T>
struct State<T&> {
    bool isValueGot;
    bool isExceptionGot;
    std::exception_ptr exception;
    T* value;
    std::mutex mut;
    std::condition_variable cv;
    State() : isValueGot(false), isExceptionGot(false) { }
    ~State() = default;
};

#endif

#ifndef _FUTURE_H
#define _FUTURE_H

#include "state.h"

static int num = 0;

template<typename T>
class Future {

public:
    Future(std::shared_ptr<State<T>> state) :
        state(state){ }
    Future(){}
    Future(Future<T>&&) = default;
    Future &operator=(Future &&) noexcept = default;
    ~Future() = default;
    Future(const Future &) = delete;
    friend bool operator<(const Future& l, const Future& r) {
        return l.v < r.v;
    }
    T Get() const;
    void Wait();
    bool IsReady();
private:
    int v = num++;
    std::shared_ptr<State<T>> state;
};

template<typename T>
T Future<T>::Get() const {
    std::unique_lock<std::mutex> locker(state->mut);
    state->cv.wait(locker, [&]() { return ((state->isValueGot == true) || (state->isExceptionGot == true)); });
    if (state->isExceptionGot) {
        std::rethrow_exception(state->exception);
    }
    return std::move(state->value);
}
template<typename T>
 void Future<T>::Wait() {
    std::unique_lock<std::mutex> locker(state->mut);
    state->cv.wait(locker, [&]() { return (state->isValueGot || state->isExceptionGot); });
    if (state->isExceptionGot) {
        throw state->exception;
    }
}
template<typename T>
bool Future<T>::IsReady() {
    std::unique_lock<std::mutex> locker(state->mut);
    return state->isValueGot;
}

template<typename T>
class Future<T&> {
public:
    Future(std::shared_ptr<State<T&>> state) :
        state(state){ }
    Future(){}
    Future(Future<T&>&&) = default;
    Future &operator=(Future &&) noexcept = default;
    ~Future() = default;
    Future(const Future &) = delete;
    friend bool operator<(const Future& l, const Future& r) {
        return l.v < r.v;
    }
    T& Get() const;
    void Wait();
    bool IsReady();
private:
    int v = num++;
    std::shared_ptr<State<T&>> state;
};

template<typename T>
T& Future<T&>::Get() const {
    std::unique_lock<std::mutex> locker(state->mut);
    state->cv.wait(locker, [&]() { return (state->isValueGot || state->isExceptionGot); });
    if (state->isExceptionGot) {
        std::rethrow_exception(state->exception);
    }
    return std::move(*state->value);
    }
template<typename T>
void Future<T&>::Wait() {
    std::unique_lock<std::mutex> locker(state->mut);
    state->cv.wait(locker, [&]() { return (state->isValueGot || state->isExceptionGot); });
    if (state->isExceptionGot) {
        throw state->exception;
    }
}
template<typename T>
bool Future<T&>::IsReady() {
    std::unique_lock<std::mutex> locker(state->mut);
    return state->isValueGot;
}
template<>
class Future<void> {
public:
    Future(std::shared_ptr<State<void>> state) :
        state(state){ }
    Future(){}
    Future(Future<void>&&) = default;
    Future &operator=(Future &&) noexcept = default;
    ~Future() = default;
    Future(const Future &) = delete;
    friend bool operator<(const Future& l, const Future& r) {
        return l.v < r.v;
    }
    void Get() const {
        std::unique_lock<std::mutex> locker(state->mut);
        state->cv.wait(locker, [&]() { return (state->isValueGot || state->isExceptionGot); });
        if (state->isExceptionGot) {
            std::rethrow_exception(state->exception);
        }
    }
    void Wait() {
        std::unique_lock<std::mutex> locker(state->mut);
        state->cv.wait(locker, [&]() { return (state->isValueGot || state->isExceptionGot); });
        if (state->isExceptionGot) {
            throw state->exception;
        }
    }
    bool IsReady() {
        std::unique_lock<std::mutex> locker(state->mut);
        return state->isValueGot;
    }
private:
    int v = num++;
    std::shared_ptr<State<void>> state;
};



#endif

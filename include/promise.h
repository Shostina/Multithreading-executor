#ifndef _PROMISE_H
#define _PROMISE_H
#include "state.h"
#include "future.h"

template<typename T>
class Promise
{
public:
    Promise()
        : state(new State<T>()) {}
    Promise(Promise<T>&&) = default;
    ~Promise() = default;

    Promise(const Promise &) = delete;
    Promise &operator=(Promise &&) noexcept = default;
    void Set(const T & value);
    void Set(Future<T> && value);
    void Set(T && value);
    void SetException(std::exception_ptr e);
    Future<T> GetFuture();
private:
    std::shared_ptr<State<T>> state;
};

template<typename T>
void Promise<T>::Set(const T & value)
{
    std::unique_lock<std::mutex> locker(state->mut);
    if (state->isValueGot == true) {
        state->isExceptionGot = true;
        throw std::logic_error("Exception: Set has already been done\n");
    }
    state->isValueGot = true;
    state->value = value;
    locker.unlock();
    state->cv.notify_all();
}
template<typename T>
void Promise<T>::Set(Future<T> && value)
{
    std::unique_lock<std::mutex> locker(state->mut);
    if (state->isValueGot == true) {
        state->isExceptionGot = true;;
        throw std::logic_error("Exception: Set has already been done\n");
    }
    state->isValueGot = true;
    (state->value) = (std::move(value));
    locker.unlock();
    state->cv.notify_all();
}
template<typename T>
void Promise<T>::Set(T && value)
{
    std::unique_lock<std::mutex> locker(state->mut);
    if (state->isValueGot) {
        state->isExceptionGot = true;
        throw std::logic_error("Exception: Set has already been done\n");
    }
    state->isValueGot = true;
    (state->value) = (std::move(value));
    locker.unlock();
    state->cv.notify_all();
}

template<typename T>
void Promise<T>::SetException(std::exception_ptr e)
{
    std::unique_lock<std::mutex> locker(state->mut);
    state->isExceptionGot = true;
    state->exception = e;
    locker.unlock();
    state->cv.notify_all();
}
template<typename T>
Future<T> Promise<T>::GetFuture()
{
    std::unique_lock<std::mutex> locker(state->mut);
    return Future<T>(state);
}

template<typename T>
class Promise<T&>
{
public:
    Promise()
        : state(new State<T&>()) {}
    Promise(Promise<T&>&&) = default;
    ~Promise() = default;
    Promise(const Promise &) = delete;
    void Set(const T & value);
    void SetException(std::exception_ptr e);
    Future<T&> GetFuture();
private:
    std::shared_ptr<State<T&>> state;

};
template<typename T>
void Promise<T&>::Set(const T & value)
{
    std::unique_lock<std::mutex> locker(state->mut);
    if (state->isValueGot) {
        state->isExceptionGot = true;
        //throw std::exception();
        throw std::logic_error("Exception: Set has already been done\n");
    }
    state->isValueGot = true;
    state->value = &value;
    locker.unlock();
    state->cv.notify_all();
}

template<typename T>
void Promise<T&>::SetException(std::exception_ptr e) {
    std::unique_lock<std::mutex> locker(state->mut);
    state->isExceptionGot = true;
    state->exception = e;
    locker.unlock();
    state->cv.notify_all();
}

template<typename T>
Future<T&> Promise<T&>::GetFuture() {
    std::unique_lock<std::mutex> locker(state->mut);
    return Future<T>(state);
}

template<>
class Promise<void>
{
public:
    Promise()
        : state(new State<void>()) {}
    Promise(Promise<void>&&) = default;
    ~Promise() = default;
    Promise(const Promise &) = delete;
    void Set()
    {
        std::unique_lock<std::mutex> locker(state->mut);
        if (state->isValueGot) {
            throw std::logic_error("Exception: Set has already been done");
        }
        state->isValueGot = true;
        (state->cv).notify_one();
    }
    void SetException(std::exception_ptr e)
    {
        std::unique_lock<std::mutex> locker(state->mut);
        state->isExceptionGot = true;
        state->exception = e;
        locker.unlock();
        state->cv.notify_all();
    }
    Future<void> GetFuture()
    {
        std::unique_lock<std::mutex> locker(state->mut);
        if (state->isValueGot) {
            state->isExceptionGot = true;
            //throw std::exception();
            throw std::logic_error("Exception: Set has already been done\n");
        }
        state->isValueGot = true;
        locker.unlock();
        state->cv.notify_all();
    }
private:
    std::shared_ptr<State<void>> state;
};
#endif

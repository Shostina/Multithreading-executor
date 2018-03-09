#ifndef _FLATTEN_
#define _FLATTEN_
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
#include <tuple>
#include "future.h"
#include "promise.h"

template<typename T> struct inner_type {
    typedef T type;

    static type get_inner_value(T const &o) {
        return o;
    }
};

template<typename T> struct inner_type<Future<T>> {
    typedef typename inner_type<T>::type type;

    static type get_inner_value(Future<T> const &o) {
        return inner_type<T>::get_inner_value(o.Get());
    }
};

template<typename T>
typename inner_type<T>::type
get_inner_value(T const &o) {
    return inner_type<T>::get_inner_value(o);
}

template<class Tuple, std::size_t... indexes>
auto cut_seq(const Tuple& t, std::index_sequence<indexes...>) {
    return std::make_tuple(std::get<1 + indexes>(t)...);
}

template<class Tuple>
auto cut_first(Tuple const &t) {
    return cut_seq(t, std::make_index_sequence<std::tuple_size<Tuple>::value - 1>());
}

template<class T>
typename std::tuple<typename inner_type<T>::type>
fl_tuple(std::tuple<T> const &t) {
    return std::make_tuple(get_inner_value(std::get<0>(t)));
}

template<class... T>
Future<std::tuple<typename inner_type<T>::type...>>
Flatten(std::tuple<T...> const &t) {
    typedef std::tuple<typename inner_type<T>::type...> R;
    Promise<R> promise;
    Future<R> future = promise.GetFuture();
    std::thread thr([&t](Promise<R> promise1) {
        try {
            R result = fl_tuple(t);
            promise1.Set(result);
        }
        catch (...) {
            promise1.SetException(std::current_exception());
        }
    }, std::move(promise));
    thr.detach();
    return future;
}

    template<typename T>
    Future<T> flatten_sync(const Future<T> &fut) {
        return std::move(const_cast<Future<T> &>(fut));
    }

    template<typename T, typename inner_type_t = typename inner_type<T>::type>
    Future<inner_type_t> flatten_sync(const Future<Future<T>> &fut) {
            auto f = fut.Get();
            return flatten_sync(f);
    }

    template<typename T>
    Future<T> Flatten(const Future<T> &fut) {
        return std::move(const_cast<Future<T> &>(fut));
    }

    template<typename T, typename inner_type_t = typename inner_type<T>::type>
    Future<inner_type_t> Flatten(const Future<Future<T>> &fut) {
        Promise<inner_type_t> promise;
        Future<inner_type_t> future = promise.GetFuture();

        std::thread t(
            [&fut](Promise<inner_type_t> promise) {
                    Future<inner_type_t> val = flatten_sync(fut);
                    try {
                        promise.Set(val.Get());
                    }
                    catch (...) {
                        promise.SetException(std::current_exception());
                    }

            }, std::move(promise)
        );

        t.detach();

        return future;
    }

    template<template <typename, typename...> class C, typename T>
    Future<C<T>> Flatten(const C<Future<T>> & f) {
        Promise<C<T>> pr;
        Future<C<T>> fut = pr.GetFuture();
        std::thread t([&f](Promise<C<T>> pr) {
            C<T> s;
            for (auto cur = f.begin(); cur != f.end(); cur++) {
                try {
                    T a = (*cur).Get();
                    s.insert(s.end(), a);
                }
                catch (...) {
                    pr.SetException(std::current_exception());
                }

            }
            try {
            pr.Set(std::move(s));
            }
            catch (...) {
                pr.SetException(std::current_exception());
            }
        }, std::move(pr));
        t.detach();
        return fut;
    }

#endif

#ifndef _MAP_H
#define _MAP_H
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

#include "promise.h"
#include "thread_pool.h"

template<typename T, typename F, typename RT = typename std::result_of<F(T)>::type>
Future<RT> Map(Future<T> & future1, F func) {
    Promise<RT>promise_map;
    std::shared_ptr<Promise<RT>> pr = std::make_shared<Promise<RT>>(std::move(promise_map));
    auto ff = pr->GetFuture();
    if(!ins){
        std::thread t([&func, &future1, pr](){
            auto a = func(future1.Get());
            pr->Set(a);
        });
        t.detach();
    } else {
        ins->execute([&func, &future1, pr]() {
            auto a = func(future1.Get());
            pr->Set(a);
        });
    }
    return ff;
}

#endif

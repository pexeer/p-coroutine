// Copyright (c) 2017, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#pragma once

#include <atomic>               // std::atomic
#include <mutex>                // std::mutex, std::unique_lock
#include <condition_variable>    // std::condition_variable
#include "p/base/macros.h"
#include "p/base/logging.h"
#include "p/thread/task.h"

namespace p {
namespace thread {

class TaskWorker;
class TaskHandle;

class WorkerManager {
public:
    constexpr static size_t kMaxWorkerNum = 64;

    WorkerManager(int work_number);

    void add_task_worker();

    uint64_t new_task(void* (*func)(void*), void* arg, uint64_t attr);

    ~WorkerManager() {}

    uint64_t worker_size() const {
        return worker_size_.load(std::memory_order_acquire);
    }

    TaskWorker* const* worker_list() const {
        return worker_list_;
    }

    uint64_t signal_pending() const {
        return signal_pending_.load(std::memory_order_acquire);
    }

    void signal_task(int number) {
#if !defined(P_OS_LINUX)
        std::unique_lock<std::mutex>    lock_gaurd(mutex_);
        condition_.notify_one();
        signal_pending_ += number;
#else
        signal_pending_.fetch_add(number, std::memory_order_release);
        futex_wake((int*)&signal_pending_, number);
#endif
    }

    void waiting_task(uint64_t signal_pending) {
#if !defined(P_OS_LINUX)
        std::unique_lock<std::mutex>    lock_gaurd(mutex_);
        if (signal_pending == signal_pending_) {
            condition_.wait(lock_gaurd);
        }
#else
        futex_wait((int*)&signal_pending_, signal_pending_, nullptr);
#endif
    }

private:
    TaskWorker*             worker_list_[kMaxWorkerNum];
    std::atomic<size_t>     worker_size_;

#if !defined(P_OS_LINUX)
    std::mutex                  mutex_;
    std::condition_variable     condition_;
#endif

    std::mutex                  add_worker_mutex_;

    P_CACHELINE_ALIGNMENT std::atomic<uint64_t>       signal_pending_;
private:
    P_DISALLOW_COPY(WorkerManager);
};

} // end namespace thread
} // end namespace p


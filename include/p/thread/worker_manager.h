// Copyright (c) 2017, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#pragma once

#include <atomic>               // std::atomic
#include <mutex>                // std::mutex, std::unique_lock
#include <condition_variable>    // std::condition_variable
#include "p/base/macros.h"
#include "p/base/logging.h"
#include "p/base/port.h"
#include "p/thread/task.h"

namespace p {
namespace thread {

class TaskWorker;
struct TaskHandle;

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

    int signal_pending() const {
        return signal_pending_.load(std::memory_order_acquire);
    }

    void futex_wake(int signal_number) {
#if !defined(P_OS_LINUX)
        std::unique_lock<std::mutex>    lock_gaurd(mutex_);
        condition_.notify_one();
        signal_pending_ += signal_number;
#else
        signal_pending_.fetch_add(signal_number, std::memory_order_release);
        p::base::futex_wake((int*)&signal_pending_, signal_number);
#endif
    }

    void futex_wait(int signal_pending) {
#if !defined(P_OS_LINUX)
        std::unique_lock<std::mutex>    lock_gaurd(mutex_);
        if (signal_pending == signal_pending_) {
            condition_.wait(lock_gaurd);
        }
#else
        p::base::futex_wait((int*)&signal_pending_, signal_pending_, nullptr);
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

    // shared with other thread
    P_CACHELINE_ALIGNMENT std::atomic<int>       signal_pending_;
private:
    P_DISALLOW_COPY(WorkerManager);
};

} // end namespace thread
} // end namespace p


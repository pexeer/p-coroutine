// Copyright (c) 2017, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#pragma once

#include <atomic>               // std::atomic
#include <mutex>                // std::mutex, std::unique_lock
#include <condition_variable>    // std::condition_variable
#include "p/base/macros.h"
#include "p/base/logging.h"

namespace p {
namespace thread {

class TaskWorker;
class TaskHandle;

class TaskManager {
public:
    constexpr static size_t kMaxWorkerNum = 64;

    TaskManager(int work_number);

    void add_task_worker();

    uint64_t new_task(void* (*func)(void*), void* arg, uint64_t attr);

    ~TaskManager() {
    }

    size_t steal_task(TaskHandle* dest[], size_t max_size, size_t* seed);

    void signal_task(int number) {
        std::unique_lock<std::mutex>    lock_gaurd(mutex_);
        condition_.notify_one();
        signal_pending_ += number;
    }

    void waiting_task(uint64_t signal_pending) {
        std::unique_lock<std::mutex>    lock_gaurd(mutex_);
        if (signal_pending == signal_pending_) {
            condition_.wait(lock_gaurd);
        }
    }
private:
    TaskWorker* worker_list_[kMaxWorkerNum];
    std::atomic<size_t> worker_size_;

    std::mutex                  mutex_;
    std::atomic<uint64_t>       signal_pending_;
    std::condition_variable     condition_;

    std::mutex                  add_worker_mutex_;

private:
    P_DISALLOW_COPY(TaskManager);
};

} // end namespace thread
} // end namespace p


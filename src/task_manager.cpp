// Copyright (c) 2017, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#include "p/thread/task_manager.h"
#include "p/thread/task_worker.h"
#include "p/base/rand.h"

namespace p {
namespace thread {

TaskManager::TaskManager(int work_number) : worker_size_{0} {
    for (int i = 0; i < work_number; ++i) {
        add_task_worker();
    }
}

void TaskManager::add_task_worker() {
    TaskWorker* w = new TaskWorker(this);

    std::lock_guard<std::mutex> lock_guard(add_worker_mutex_);

    worker_list_[worker_size_] = w;
    ++worker_size_;
}

size_t TaskManager::steal_task(TaskHandle* dest[], size_t max_size, size_t* seed) {
    size_t const worker_size = worker_size_;
    if (worker_size <= 1) {
        return 0;
    }

    size_t offset = 0;
    do {
        offset = base::fast_rand(worker_size);
    } while (offset && ((worker_size % offset) == 0));

    size_t ret = 0;
    for (size_t i = 0; i < worker_size; ++i, *seed += offset) {
        TaskWorker* worker = worker_list_[*seed % worker_size];
        ret = worker->steal_task(dest, max_size);
        if (ret) {
            return ret;
        }
    }

    return ret;
}

void TaskManager::signal_task() {
    std::unique_lock<std::mutex>    lock_gaurd(mutex_);
    condition_.notify_one();
}

void TaskManager::waiting_task() {
    std::unique_lock<std::mutex>    lock_gaurd(mutex_);
    condition_.wait(lock_gaurd);
}

uint64_t TaskManager::new_task(void* (*func)(void*), void* arg, uint64_t attr) {
    TaskWorker* w = TaskWorker::tls_w;
    if (!w) {
        size_t const worker_size = worker_size_;
        w = worker_list_[base::fast_rand(worker_size)];
    }
    return w->new_task(func, arg, attr);
}

} // end namespace thread
} // end namespace p

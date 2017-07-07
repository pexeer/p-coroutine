// Copyright (c) 2017, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#include "p/thread/worker_manager.h"
#include "p/thread/task_worker.h"
#include "p/base/rand.h"

namespace p {
namespace thread {

WorkerManager::WorkerManager(int work_number) : worker_size_{0} {
    for (int i = 0; i < work_number; ++i) {
        add_task_worker();
    }
}

void WorkerManager::add_task_worker() {
    std::lock_guard<std::mutex> lock_guard(add_worker_mutex_);
    TaskWorker* w = new TaskWorker(this, worker_size_);
    worker_list_[worker_size_] = w;
    worker_size_.fetch_add(1, std::memory_order_release);
}

uint64_t WorkerManager::new_task(void* (*func)(void*), void* arg, uint64_t attr) {
    TaskWorker* w = TaskWorker::tls_w;
    if (!w) {
        size_t const worker_size = worker_size_.load(std::memory_order_release);
        w = worker_list_[base::fast_rand(worker_size)];
    }
    uint64_t tid = w->new_task(func, arg, attr);
    if (tid) {
        signal_task(1);
    }
    return tid;
}

} // end namespace thread
} // end namespace p

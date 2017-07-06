// Copyright (c) 2017, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#pragma once

#include <atomic>
#include <thread>
#include "p/thread/task.h"
#include "p/thread/task_manager.h"
#include "p/thread/work_stealing_queue.h"

namespace p {
namespace thread {

class TaskManager;

class TaskWorker {
public:
    TaskWorker(TaskManager* m);

    ~TaskWorker() {
    }

    uint64_t new_task(void* (*func)(void*), void* arg, uint64_t attr);

    TaskHandle* pop() {
        return task_queue_.pop();
    }

    size_t steal_task(TaskHandle** item, size_t max_size) {
        return task_queue_.steal(item, max_size);
    }

    void push_back(TaskHandle* task) {
        task_queue_.push_back(task);
    }

    TaskHandle* main_task() {
        return &main_task_;
    }

    // jump to next_task's stack context
    TaskHandle* jump_to(TaskHandle* next_task) {
        next_task_ = next_task;
        TaskHandle* from = cur_task_->jump_to(next_task_);
        return tls_w->dealing_with_from_task(from);
    }

    // not change stack context, just swith to next stack context
    TaskHandle* swith_to(TaskHandle* next_task) {
        next_task_ = next_task;
        next_task->task_stack = cur_task_->task_stack;
        cur_task_->task_stack = nullptr;
        return dealing_with_from_task(cur_task_);
    }

    TaskHandle* dealing_with_from_task(TaskHandle* from);

    void main_task_func();

    friend void StackFunc(transfer_t jump_from);

    typedef WorkStealingQueue<TaskHandle>   TaskQueue;
public:
    static bool running_in_worker() {
        return tls_w != nullptr;
    }

    static thread_local TaskWorker*      tls_w;
private:
    TaskManager*    task_manager_;
    TaskHandle      main_task_;
    TaskStack       main_stack_;
    TaskHandle*     next_task_;
    TaskHandle*     cur_task_;
    size_t          seed_;

    std::thread     thread_;

    // shared with other thread
    P_CACHELINE_ALIGNMENT TaskQueue       task_queue_;

    P_DISALLOW_COPY(TaskWorker);
};

} // end namespace thread
} // end namespace p


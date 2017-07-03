// Copyright (c) 2017, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#pragma once

#include <atomic>
#include <thread>
#include "p/thread/task.h"
#include "p/thread/work_stealing_queue.h"

namespace p {
namespace thread {

class TaskManager;

extern void StackFunc(transfer_t jump_from);

class TaskWorker {
public:
    TaskWorker(TaskManager* m);

    ~TaskWorker() {
    }

    TaskHandle* pop() {
        return task_queue_.pop();
    }

    void push_back(TaskHandle* task) {
        task_queue_.push_back(task);
    }

    TaskHandle* main_task() {
        return &main_task_;
    }

    TaskHandle* jump_to(TaskHandle* next_task) {
        next_task_ = next_task;
        return cur_task_->jump_to(next_task_);
    }

    TaskHandle* jump_to() {
        jump_to(main_task());
    }

    //new_task();

    //prepare_context();

    friend void StackFunc(transfer_t jump_from);

    typedef WorkStealingQueue<TaskHandle>   TaskQueue;
public:
    static bool running_in_worker() {
        return tls_w != nullptr;
    }

    static thread_local TaskWorker*      tls_w;
private:
    TaskHandle      main_task_;
    TaskStack       main_stack_;
    TaskHandle*     next_task_;
    TaskHandle*     cur_task_;

    std::thread     thread_;

    // shared with other thread
    P_CACHELINE_ALIGNMENT TaskQueue       task_queue_;
};

} // end namespace thread
} // end namespace p


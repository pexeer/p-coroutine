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

class TaskWorker {
public:
    TaskWorker(TaskManager* m);

    ~TaskWorker() {
    }

    //new_task();

    //prepare_context();

    typedef WorkStealingQueue<TaskHandle>   TaskQueue;
public:
    static bool running_in_worker() {
        return tls_w != nullptr;
    }

    static thread_local TaskWorker*      tls_w;
private:
    TaskHandle      main_task_;
    TaskStack       main_stack_;
    TaskHandle*     pre_task_;
    TaskHandle*     cur_task_;


    std::thread     thread_;

    // shared with other thread
    P_CACHELINE_ALIGNMENT TaskQueue       task_queue_;
};

} // end namespace thread
} // end namespace p


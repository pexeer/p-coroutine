// Copyright (c) 2017, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#include "p/thread/task_worker.h"
#include "p/thread/task_manager.h"
#include "p/base/rand.h"

namespace p {
namespace thread {

thread_local TaskWorker* TaskWorker::tls_w = nullptr;

TaskWorker::TaskWorker(TaskManager* m) : task_manager_(m) {
    seed_ = base::fast_rand();

    thread_ = std::move(std::thread(&TaskWorker::main_task_func, this));
}

uint64_t TaskWorker::new_task(void* (*func)(void*), void* arg, uint64_t attr) {
    TaskHandle* task = TaskHandle::pop();
    task->func = func;
    task->arg = arg;
    task->flag = 0;
    uint64_t ret = task->tid;
    if (tls_w == this && attr) {
        jump_to(task);
    } else {
        push_back(task);
    }
    return ret;
}

inline TaskHandle* TaskWorker::dealing_with_from_task(TaskHandle* from) {
    //LOG_INFO << "dealing_with_from_task=" << from << ",tid=" << from->tid;
    assert(from == cur_task_);
    cur_task_ = next_task_;

    if (from !=  main_task()) {
        if (from->flag == 0x3) {
            //LOG_INFO << "Task " << from->tid << " run finished";
            TaskHandle::push(from);
        } else {
            //LOG_INFO << "task yield tid=" << from->tid << " task=" << from;
            push_back(from);
        }
    }
    return cur_task_;
}

void normal_task_func(transfer_t jump_from) {
    TaskWorker* w = TaskWorker::tls_w;
    TaskHandle* from = (TaskHandle*)(jump_from.data);
    from->task_stack->pcontext = jump_from.fctx;
    //LOG_INFO << "fillback pcontext=" << jump_from.fctx << ",task=" << from
        //<< ",tid=" << from->tid;

    TaskHandle* cur_task = w->dealing_with_from_task(from);
    do {
        cur_task->func(cur_task->arg); // maybe change TaskWorker
        cur_task->flag = 0x3;

        // refresh current TaskWorker
        w = TaskWorker::tls_w;

        // current task running finished, get another task from TaskWorker
        TaskHandle* next_task = w->pop();
        if (next_task) {
            if (next_task->task_stack == nullptr) {
                cur_task = w->swith_to(next_task);
                continue;
            }
        } else {
            // jump to main pthread
            next_task = w->main_task();
        }

        // jump to next task
        cur_task = w->jump_to(next_task);
    } while (true);
}

void TaskWorker::main_task_func() {
    //LOG_INFO << "new worker start, worker=" << (void*)this;
    main_task_.tid = 0;
    main_task_.task_stack = &main_stack_;
    main_task_.func = nullptr;
    main_task_.arg = nullptr;

    cur_task_ = &main_task_;
    next_task_ = nullptr;
    TaskWorker::tls_w = this;

    TaskHandle* task_list[4096];
    TaskHandle* jump_from;
    size_t task_number = 0;
    while (true) {
        next_task_ = task_queue_.pop();
        if (next_task_) {
            jump_from = jump_to(next_task_);
            jump_from = dealing_with_from_task(jump_from);
            assert(jump_from == main_task());
            continue;
        }

        // stealing task from task_manager
        task_number = task_manager_->steal_task(task_list, 4096, &seed_);
        task_queue_.push_back(task_list, task_number);
    }
}

} // end namespace thread
} // end namespace p


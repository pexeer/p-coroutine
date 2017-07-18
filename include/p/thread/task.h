// Copyright (c) 2017, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#pragma once

#include <unistd.h>                 // size_t
#include <stdint.h>                 // uint64_t
#include "p/thread/pcontext.h"      // jump_pcontext
#include "p/base/object_pool.h"     // ObjectPool<>
#include "p/base/macros.h"

namespace p {
namespace thread {

struct TaskHandle;

extern void normal_task_func(transfer_t jump_from);

struct TaskStack {
public:
    constexpr static size_t kTaskStackSize = 32 * 4096;

    TaskStack(size_t size);

    TaskStack() : stack_size(0), pcontext(nullptr) {}

    static TaskStack* NewThis();

    static void FreeThis(TaskStack* ts);

    const size_t    stack_size;
    void*           pcontext;
    const char      stack_guard[8] = {'T', 'A', 'S', 'K', 'T', 'A', 'S', 'K'};

    P_DISALLOW_COPY(TaskStack);
};

struct P_CACHELINE_ALIGNMENT TaskHandle {
    uint64_t tid;
    uint64_t flag;
    void* (*func)(void*);
    void* arg;
    TaskStack* task_stack;
public:
    typedef base::ArenaObjectPool<TaskHandle>   TaskHandleFactory;
    typedef base::ObjectPool<TaskStack>         TaskStackFactory;

    // static method
    static TaskHandle* pop() {
        uint64_t obj_id;
        TaskHandle* ret = TaskHandleFactory::get(&obj_id);
        ret->tid = obj_id + 0x100000000;
        return ret;
    }

    static void push(TaskHandle* task) {
        task->tid += 0x100000000;
        if (task->task_stack) {
            // return task_stack
            TaskStackFactory::put(task->task_stack);
            task->task_stack = nullptr;
        }
        // return task_handle
        TaskHandleFactory::put(task->tid);
    }

public:
    TaskHandle() : func(nullptr), task_stack(nullptr) {}

    TaskHandle* jump_to(TaskHandle* next_task) {
        if (next_task->task_stack == nullptr) {
            next_task->task_stack = TaskStackFactory::get();
        }

        auto tmp = next_task->task_stack->pcontext;

        //LOG_INFO << "jump_to pcontext=" << tmp << ",task=" << next_task
        //    << ",tid=" << next_task->tid << ",this=" << this;
        transfer_t jump_from = jump_pcontext(tmp, this);
        //transfer_t jump_from = jump_pcontext(next_task->task_stack->pcontext, this);
        TaskHandle* from = (TaskHandle*)(jump_from.data);
        from->task_stack->pcontext = jump_from.fctx;
        //LOG_INFO << "fillback pcontext=" << jump_from.fctx << ",task=" << from
        //    << ",tid=" << from->tid;
        return from;
    }

    TaskHandle* swith_to(TaskHandle* next_task) {
        if (next_task->task_stack == nullptr) {
            next_task->task_stack = task_stack;
            task_stack = nullptr;
        }
        return this;
    }

private:
    P_DISALLOW_COPY(TaskHandle);
};

} // end namespace thread
} // end namespace p


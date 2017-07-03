// Copyright (c) 2017, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#include "p/thread/task_worker.h"

namespace p {
namespace thread {

thread_local TaskWorker* TaskWorker::tls_w = nullptr;

void StackFunc(transfer_t jump_from) {
    TaskWorker* w = nullptr;
    TaskHandle* from = (TaskHandle*)(jump_from.data);
    from->task_stack->pcontext = jump_from.fctx;

    do {
        w = TaskWorker::tls_w;
        assert(from == w->cur_task_);
        w->cur_task_ = w->next_task_;

        // dealing with jump_from TaskHandle
        if (from != w->main_task()) {
            if (from->flag == 0x3) {
                LOG_TRACE << "Task " << from->tid << " run finished";
                TaskHandle::destroy(from);
            } else {
                w->push_back(from);
            }
        }

        TaskHandle* const cur_task = w->next_task_;
        cur_task->func(cur_task->arg); // maybe change TaskWorker
        cur_task->flag = 0x3;

        // refresh current TaskWorker
        w = TaskWorker::tls_w;
        // current task running finished, get another task from TaskWorker
        TaskHandle* next_task = w->pop();
        if (next_task) {
            // jump to next task
            from = w->jump_to(next_task);
        } else {
            // jump to main task
            from = w->jump_to();
        }
    } while (true);
}


} // end namespace thread
} // end namespace p


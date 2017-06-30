#include "p/thread/task.h"

namespace p {
    namespace thread {
TaskHandle* main_task;

void StackFunc(transfer_t jump_from) {
    JumpContext* ret = (JumpContext*)(jump_from.data);
    ret->from->task_stack->pcontext = jump_from.fctx;

    do {
        if (ret->from == main_task) {
            printf("come from main task\n");
        }
        TaskHandle * run = ret->to;
        printf("run func %p for arg %p\n", run->func, run->arg);
        run->func(run->arg);
        printf("end of run\n");
        ret = ret->from->jump(run);
    } while (true);
}

}}

using namespace p::thread;


void* f(void* fuck) {
    printf("function f called, argv=%p\n", fuck);
    return nullptr;
}

int main() {
    p::thread::TaskHandle* mt = p::thread::TaskHandle::build();
    mt->task_stack = new TaskStack();
    main_task = mt;

    TaskHandle* t2 = TaskHandle::build();
    t2->func = f;
    t2->arg = (void*)23456;

    for (int i = 0; i < 3; ++i) {
        JumpContext* ret =  t2->jump(mt);
        if (ret->from == main_task) {
            printf("come from main task\n");
        }
    }
    printf("end\n");
    return 0;
}

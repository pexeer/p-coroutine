// Copyright (c) 2017, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#include "p/thread/task.h"

#include <assert.h>
#include <sys/mman.h>
#include "p/base/process.h"

namespace p {
namespace thread {

TaskStack::TaskStack(size_t size) : stack_size(size) {
    pcontext = make_pcontext((void*)this, stack_size, normal_task_func);
}

TaskStack* TaskStack::NewThis() {
    const uint64_t real_stack_size = ((kTaskStackSize - 1) /
            base::Process::kPageSize + 1) * base::Process::kPageSize;

#if defined(P_NO_STACK_GUARD)
    char* const mem_ptr = (char*)::malloc(real_stack_size);
#else
    char* const mem = (char*)::mmap(nullptr, real_stack_size + base::Process::kPageSize,
        (PROT_READ | PROT_WRITE), (MAP_ANON | MAP_PRIVATE), -1, 0);

    if (MAP_FAILED == mem) {
        return nullptr;
    }

    if (::mprotect(mem, base::Process::kPageSize, PROT_NONE)) {
        ::munmap(mem, real_stack_size);
        return nullptr;
    }
    char* const mem_ptr = mem + base::Process::kPageSize;
#endif // P_NO_STACK_GUARD

    char* end_mem_ptr = mem_ptr + real_stack_size;
    char* tc_address = (end_mem_ptr - sizeof(TaskStack));
    return new(tc_address) TaskStack(real_stack_size - sizeof(TaskStack));
}

void TaskStack::FreeThis(TaskStack* tc) {
    char* tc_address = (char*)(tc);
    char* mem_ptr = tc_address - tc->stack_size;

#if defined(P_NO_STACK_GUARD)
    ::free(mem_ptr);
#else
    mem_ptr -= base::Process::kPageSize;
    ::munmap(mem_ptr, tc->stack_size + sizeof(TaskStack) + base::Process::kPageSize);
#endif // P_NO_STACK_GUARD
}

} // end namespace thread
} // end namespace p


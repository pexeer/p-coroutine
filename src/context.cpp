// Copyright (c) 2017, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#include "context.h"

#include <unistd.h>
#include <assert.h>
#include <sys/mman.h>
#include "p/base/stack.h"
#include "p/base/process.h"

namespace p {
namespace thread {

p::base::ObjectPool<ThreadContext>   g_thread_context_pool;

void fProc(transfer_t x) {
    while (1) {
        LOG_TRACE << "fProc return from jump_pcontext, fctx=" << x;
        ThreadContext* tc = static_cast<ThreadContext*>(x.data);
        tc->func_();
        x = jump_pcontext(x.fctx, tc);
    }
}

constexpr static uint64_t kThreadContextStackSize = 128 * 4096;
constexpr uint64_t kSizeOfThreadContext = sizeof(ThreadContext);

ThreadContext* ThreadContext::NewThis() {
    const uint64_t real_stack_size = ((kThreadContextStackSize - 1) /
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
    char* tc_address = (end_mem_ptr - kSizeOfThreadContext);
    return new(tc_address) ThreadContext(real_stack_size - kSizeOfThreadContext);
}

void ThreadContext::FreeThis(ThreadContext* tc) {
    char* tc_address = (char*)(tc);
    char* mem_ptr = tc_address - tc->stack_size_;
#if defined(P_NO_STACK_GUARD)
    ::free(mem_ptr);
#else
    mem_ptr -= base::Process::kPageSize;
    ::munmap(mem_ptr, tc->stack_size_ + kSizeOfThreadContext + base::Process::kPageSize);
#endif // P_NO_STACK_GUARD
}

constexpr static int kThreadContextGroupItemSize = 256;

struct ThreadContextGroup {
    ThreadContextGroup*     next;
    int                     size = 0;
    ThreadContext*          list[kThreadContextGroupItemSize];
};

static p::base::LinkedStack<ThreadContextGroup>    global_full_chunk_queue;
static p::base::LinkedStack<ThreadContextGroup>    global_free_chunk_queue;

class ThreadLocalGroupPtr {
public:
    ThreadContext* get() {
        if (group_ptr_) {
            if (group_ptr_->size > 0) {
                return group_ptr_->list[--group_ptr_->size];
            }
            global_free_chunk_queue.push(group_ptr_);
        }
        group_ptr_ = global_full_chunk_queue.pop();

        if (group_ptr_) {
            assert(group_ptr_->size > 0 && group_ptr_->size <= kThreadContextGroupItemSize);
            --(group_ptr_->size);
            return group_ptr_->list[group_ptr_->size];
        }

        return ThreadContext::NewThis();
    }

    void put(ThreadContext* tc) {
        if (group_ptr_) {
            if (group_ptr_->size < kThreadContextGroupItemSize) {
                group_ptr_->list[(group_ptr_->size)++] = tc;
                return ;
            } else {
                global_full_chunk_queue.push(group_ptr_);
            }
        }

        group_ptr_ = global_free_chunk_queue.pop();
        if (!group_ptr_) {
            group_ptr_ = new(std::nothrow) ThreadContextGroup;
        }
        group_ptr_->size = 1;
        group_ptr_->list[0] = tc;
    }

    ~ThreadLocalGroupPtr() {
        if (group_ptr_) {
            global_full_chunk_queue.push(group_ptr_);
        }
    }

private:
    ThreadContextGroup*     group_ptr_ = nullptr;
};

thread_local static ThreadLocalGroupPtr tls_group_ptr;

ThreadContext* ThreadContextFactory::get() {
    return tls_group_ptr.get();
}

void ThreadContextFactory::put(ThreadContext* tc) {
    return tls_group_ptr.put(tc);
}

} // end namespace thread
} // end namespace p


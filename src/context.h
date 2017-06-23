// Copyright (c) 2017, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#pragma once
#include <stdint.h>
#include "p/base/logging.h"
#include "p/thread/pcontext.h"

namespace p {
namespace thread {

extern void fProc(transfer_t caller);

struct ThreadContext {
public:
    static ThreadContext* NewThreadContext();

    static void FreeThreadContext(ThreadContext*);

    ThreadContext* jump(void (*f)()) {
        func_ = f;
        transfer_t x = jump_pcontext(context_, this);
        LOG_TRACE << "jump return from jump_pcontext, fctx=" << x.fctx << ",data=" << x.data;
        context_ = x.fctx;
        return static_cast<ThreadContext*>(x.data);
    }
private:
    ThreadContext(uint64_t stack_size) : stack_size_(stack_size) {
        context_ = make_pcontext((void*)this, stack_size_, fProc);
    }

public:
    uint64_t const stack_size_;
    void*       context_;
    void       (*func_)();
};

class ThreadContextFactory {
    static ThreadContext* get();

    static void put(ThreadContext* tc);
};

} // end namespace thread
} // end namespace p

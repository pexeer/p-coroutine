// Copyright (c) 2017, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#pragma once

#include <stdint.h>
#include <functional>
#include "p/base/logging.h"
#include "p/base/object_pool.h"
#include "p/thread/pcontext.h"

namespace p {
namespace thread {

inline p::base::LogStream& operator<<(p::base::LogStream& ls, const transfer_t& tf) {
    return ls << '(' << tf.fctx << ',' << tf.data << ')';
}

extern void fProc(transfer_t caller);

struct ThreadContext {
public:
    static ThreadContext* NewThis();

    static void FreeThis(ThreadContext*);

    ThreadContext* jump(const std::function<void()>& f) {
        func_ = f;
        transfer_t x = jump_pcontext(context_, this);
        LOG_TRACE << "jump return from jump_pcontext, tf=" << x;
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
    std::function<void()>   func_;
};

class ThreadContextFactory {
    static ThreadContext* get();

    static void put(ThreadContext* tc);
};

extern p::base::ObjectPool<ThreadContext>   g_thread_context_pool;

} // end namespace thread
} // end namespace p

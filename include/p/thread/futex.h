// Copyright (c) 2017, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#pragma once

namespace p {
namespace thread {

struct Waiter {
    Waiter*         next;
    TaskHandle*     task;   // waiter in pthread when task == nullptr
    std::atomic<int32_t>     expected_value;
};

class Futex {
public:
    Futex() : value_{0}, pv_{0}, waiters(nullptr) {}

    ~Futex() {
        assert(waiters_ == nullptr);
    }

    int futex_wait(int expected_value, const timespec* abstime);

    int futex_wake(int wake_num);

    int futex_wake();

    int futex_wait(Waiter* waiter)

private:
    std::atomic<uint32_t>    value_;
    std::atomic<uint32_t>    pv_;
    std::mutex              mutex_;
    Waiter*                 waiters_;
};

inline int Futex::futex_wait(const int expected_value, const timespec* abstime) {
    uint32_t value = value_.load(std::memory_order_acquire);
    if (expected_value != value) {
        return -1;
    }

    Waiter  waiter;
    waiter->next = nullptr;
    waiter->expected_value = expected_value;
    TaskWorker* w = TaskWorker::tls_w;
    if (w) {
        waiter->task = w->cur_task();
    } else {
        waiter->task = nullptr;
        if (futex_wait(&waiter)) {
        }
    }
    return 0;
}

inline int Futex::futex_wait(Waiter* waiter) {
    const uint32_t value = waiter->expected_value;
    {
        std::unique_lock<std::mutex_> lock_guard(mutex_);
        if (value == value_) {
            waiter->next = waiters_;
            waiters_ = waiter;
            return 0;
        }
    }
    return -1;
}

inline int Futex::futex_wake() {
    Waiter* head = nullptr;
    {
        std::unique_lock<std::mutex>    lock_guard(mutex_);
        if (waiters_) {
            head = waiters_;
            waiters_ = waiters_->next;
        }
    }

    if (head) {
        return 1;
    }

    return 0;
}

inline int Futex::futex_wake(int wake_num) {
    Waiter* head = nullptr;
    Waiter* pre_head = nullptr;

    int wake = 0;
    {
        std::unique_lock<std::mutex>    lock_guard(mutex_);
        while (wake < wake_num && waiters_) {
            head = waiters_;
            waiters_ = waiters_->next;
            head->next = pre_head;
            pre_head = head;
            ++wake;
        }
    }

    while (head) {

    }

    return wake;
}

} // end namespace thread
} // end namespace p


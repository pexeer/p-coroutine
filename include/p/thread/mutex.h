// Copyright (c) 2017, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#pragma once

struct Waiter {
    Waiter*     next;
    TaskHandle* task;
};

class Futex {
public:
    Futex() : lock_{0}, pv_{0}, waiters(nullptr) {}

private:
    std::atomic<uint32_t>    lock_;
    std::atomic<uint32_t>    pv_;
    std::mutex              mutex_;
    Waiter*                 waiters_;
};

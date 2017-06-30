// Copyright (c) 2017, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#pragma once

#include <atomic>

namespace p {
namespace thread {

class TaskWorker;

class TaskManager {
public:
    constexpr static size_t kMaxWorkerNum = 64;

    bool init();

    ~TaskManager() {

    }

private:
    TaskWorker* worker_list_[kMaxWorkerNum];
    std::atomic<size_t> worker_size_;
};

} // end namespace thread
} // end namespace p


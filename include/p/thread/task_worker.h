// Copyright (c) 2017, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#pragma once

#include <atomic>
#include <thread>

namespace p {
namespace thread {

class TaskManager;

class TaskWorker {
public:
    TaskWorker(TaskManager* m);

    ~TaskWorker() {
    }

private:

    std::thread     thread_;
};

} // end namespace thread
} // end namespace p


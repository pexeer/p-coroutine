// Copyright (c) 2017, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#include "p/thread/task_worker.h"

namespace p {
namespace thread {

thread_local TaskWorker* TaskWorker::tls_w = nullptr;


} // end namespace thread
} // end namespace p


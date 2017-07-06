#include "p/thread/task_manager.h"
#include "p/base/logging.h"
#include <unistd.h>


p::thread::TaskManager* g_tm;

void* func(void * arg) {
    uint64_t number = (uint64_t)arg;
    LOG_INFO << "task=" << number << ", started";

    if (number) {
        uint64_t tid = g_tm->new_task(func, (void*)(number-1), 1);
        LOG_INFO << "task=" << number << ", add new task, tid=" << tid;
    }

    LOG_INFO << "task=" << number << ", sleep";
    sleep(1);

    LOG_INFO << "task=" << number << ", fnished";

    return nullptr;
}

int main() {
    p::thread::TaskManager tm(4);
    g_tm = &tm;

//    tm.add_task_worker();

    uint64_t tid = tm.new_task(func, (void*)(3), 0);
    LOG_INFO << "add new task, tid=" << tid;

    sleep(10);
//  tm.signal_task();
    sleep(1000);
    return 0;
}

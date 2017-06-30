#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <thread>
#include <vector>
#include <map>
#include <mutex>

#include "src/context.h"

//extern "C" transfer_t jump_pcontext( pcontext_t const to, void * vp);
//extern "C" pcontext_t make_pcontext( void * sp, std::size_t size, void (* fn)( transfer_t) );

void f() {
    LOG_TRACE << "called f";
}

#define ITEM 5000

std::map<p::thread::ThreadContext*, int> g_map;
std::mutex                              g_mutex;

int a() {
    std::vector<p::thread::ThreadContext*> list;
    for (size_t i =0 ; i < ITEM; ++i) {
        p::thread::ThreadContext* tc = p::thread::g_thread_context_pool.get();
        if (tc) {
            list.push_back(tc);
        }
    }

    for (size_t i = 0;i < list.size(); ++i) {
        //list[i]->jump(f);
        p::thread::g_thread_context_pool.put(list[i]);
    }

    LOG_TRACE << "a_return,";

    g_mutex.lock();
    for (size_t i = 0; i < list.size(); ++i) {
        g_map[list[i]] = 0;
    }
    g_mutex.unlock();

    return 0;
}

int b() {
    while (true) {
        std::vector<p::thread::ThreadContext*> list;
        for (int i =0 ; i < 257; ++i) {
            p::thread::ThreadContext* tc = p::thread::g_thread_context_pool.get();
            list.push_back(tc);
        }

        g_mutex.lock();
        for (int i = 0; i < 257; ++i) {
            g_map.erase(list[i]);
        }

        if (g_map.size() == 0) {
            g_mutex.unlock();
            LOG_TRACE << "end worl";
            break;
        }
        g_mutex.unlock();
    }
    return 0;
}

int main() {
    std::vector<std::thread> list;
    for (size_t i = 0; i < 20; ++i) {
        list.push_back(std::thread(a));
    }
    for (size_t i = 0; i < list.size(); ++i) {
        list[i].join();
    }

    std::thread tmp(b);
    tmp.join();

    p::base::stop_logging();
    return 0;
}


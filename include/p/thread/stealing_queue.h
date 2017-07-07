// Copyright (c) 2017, pexeer@gmail.com All rights reserved.
// Licensed under a BSD-style license that can be found in the LICENSE file.

#pragma once

#include <atomic>
#include <mutex>
#include <deque>

namespace p {
namespace thread {

template<typename T>
class StealingQueue {
public:
    size_t size() const {
        return size_;
    }

    bool push_back(T* item) {
        std::unique_lock<std::mutex>   lock_guard(mutex_);
        q_.push_back(item);
        size_ += 1;
        return true;
    }

    T*  pop() {
        std::unique_lock<std::mutex>   lock_guard(mutex_);
        if (size_ > 0) {
            --size_;
            T* ret = q_.front();
            q_.pop_front();
            return ret;
        }
        return nullptr;
    }

    void push_back(T** item, size_t size) {
        std::unique_lock<std::mutex>   lock_guard(mutex_);
        for (size_t i = 0; i < size; ++i) {
            q_.push_back(item[i]);
            ++size_;
        }
    }

    size_t steal(T** item, size_t max_size) {
        size_t i = 0;
        if (size_ > 0) {
            std::unique_lock<std::mutex>   lock_guard(mutex_);
            if (size_ > 0) {
                size_t size = size_ / 2;
                if (size > max_size) {
                    size = max_size;
                }
                do {
                    --size_;
                    item[i++] = q_.back();
                    q_.pop_back();
                } while (i < size);
            }
        }
        return i;
    }

private:
    std::mutex          mutex_;
    std::atomic<size_t> size_;
    std::deque<T*>  q_;
};

} // end namespace thread
} // end namespace p


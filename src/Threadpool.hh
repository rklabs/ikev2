// Copyright (c) 2012 Jakob Progsch, Václav Zeman

// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.

// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:

// 1. The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software. If you use this software
// in a product, an acknowledgment in the product documentation would be
// appreciated but is not required.

// 2. Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.

// 3. This notice may not be removed or altered from any source distribution.

#pragma once

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <future>
#include <functional>
#include <condition_variable>

#include <sys/types.h>
#include <unistd.h>

#include "Logging.hh"

#define MAX_THREADS 50

#define ENQUEUE_TASK(...) IKEv2::ThreadPool::getThreadPool().enqueue(__VA_ARGS__)

namespace IKEv2{

class ThreadPool final {
 public:
     ThreadPool(size_t);
     ~ThreadPool();
    static ThreadPool & getThreadPool();
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args)
    -> std::future<typename std::result_of<F(Args...)>::type>;
    S32 killThread();
    S32 threadAffinityIs();
    S32 shutdown();

    // Delete all copy / move constructors
    ThreadPool(const ThreadPool &)=delete;
    ThreadPool & operator=(const ThreadPool &)=delete;

 private:
    void workerFunc();
    bool stop;
    S32 threadCount;
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;

    std::mutex queueMutex;
    std::condition_variable condition;
};

inline void
ThreadPool::workerFunc() {
    TRACE();
    // Indefinite worker loop which pops tasks from queue and
    // executes them until threadpool is deleted
    while (true) {
        std::function<void()> task;
        // Scoped lock, same lock is used to protect both tasks & stop
        {
            // Worker thread will block till new work in enqueued
            std::unique_lock<std::mutex> lock(queueMutex);
            condition.wait(lock, [this]{ return this->stop ||
                                         !this->tasks.empty(); });
            // If thread pool is deleted or tasks is empty
            if (stop || tasks.empty()) {
                return;
            }
            // Move temp task object and then pop
            task = tasks.front();
            tasks.pop();
        }
        // Do your thing
        task();
        threadCount--;
    }
}

// Constructor just launches some amount of workers
inline
ThreadPool::ThreadPool(size_t threads) : stop(false), threadCount(0) {
    TRACE();
    for(size_t i = 0 ; i < threads ; ++i) {
        LOG(INFO, "Creating thread %d", i);
        workers.emplace_back(&ThreadPool::workerFunc, this);
    }
    LOG(INFO, "All threads started successfully!");
}

// Add new work item to the pool
template <class F, class... Args>
auto
ThreadPool::enqueue(F&& f, Args&&... args) ->
    std::future<typename std::result_of<F(Args...)>::type> {
    TRACE();

    if (threadCount > MAX_THREADS) {
        LOGT("Max threads in thread pool exceeded");
        LOGT("Creating more threads on-demand");
        workers.emplace_back(&ThreadPool::workerFunc, this);
    }

    // Define return type
    using returnType = typename std::result_of<F(Args...)>::type;

    // Define generic funtion with any number / type of args
    // and return type, make it callable without args(func())
    // using std::bind
    auto func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
    auto task = std::make_shared<std::packaged_task<returnType()>>(func);

    // Get future object for obtaining return value of task
    auto res = task->get_future();

    //Scoped lock for queue
    {
        std::unique_lock<std::mutex> lock(queueMutex);

        // Don't allow enqueueing after stopping the pool
        if (stop) {
            throw std::runtime_error("enqueue on stopped ThreadPool");
        }

        // Add task to queue
        tasks.emplace([task](){ (*task)(); });
        threadCount++;
        LOG(INFO, "Successfully added task");
    }

    // Notify worker thread
    condition.notify_one();

    return res;
}

inline S32
ThreadPool::killThread() {
    TRACE();
    return 0;
}

inline S32
ThreadPool::threadAffinityIs() {
    TRACE();
    return 0;
}

// Destructor joins all threads
inline
ThreadPool::~ThreadPool() {
    TRACE();
    if (!stop) {
        shutdown();
    }
}

inline S32
ThreadPool::shutdown() {
    TRACE();
    if (!stop) {
        // Scoped lock for stop variable
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            stop = true;
        }

        // Notify all threads to stop executing
        condition.notify_all();

        // Wait for all threads to complete
        try {
            for (auto & worker: workers) {
                LOG(INFO, "Joining thread %x", worker.get_id());
                worker.join();
            }
            LOG(INFO, "Done joining all threads");
        }
        catch(const std::system_error & err) {
            LOGT("Exception while joining threads");
            LOGT("Eror code %d, meaning %s", err.code().value(), err.what());
            return -1;
        }
    }
    return 0;
}

}  // namespace IKEv2

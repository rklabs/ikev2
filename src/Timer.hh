/*
 * Copyright (C) 2014 Raju Kadam <rajulkadam@gmail.com>
 *
 * IKEv2 is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * IKEv2 is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <iostream>
#include <ctime>
#include <cstdlib>
#include <algorithm>  // std::sort
#include <thread>
#include <mutex>
#include <chrono>
#include <future>
#include <deque>
#include <vector>
#include <functional>
#include <condition_variable>
#include <memory>

#include "Logging.hh"
#include "Threadpool.hh"
#include "BasicTypes.hh"

#define ENQUEUE_TIMER_TASK(...) \
    Timer::AsyncTimer::getAsyncTimer().createTimerEvent(__VA_ARGS__);

namespace Timer {

class Event {
 public:
    Event(S32 id, S32 timeLeft,
          S32 timeout, bool repeat,
          S32 repeatCount,
          std::chrono::time_point<std::chrono::system_clock> startTime,
          std::function<void()> eventHandler);
    S32 id_;       // unique id for each event object
    S32 timeLeft_;
    S32 timeout_;  // in millisec
    bool repeat_;  // repeat event indefinitely
    S32 repeatCount_;
    // timestamp of event creation
    std::chrono::time_point<std::chrono::system_clock> startTime_;
    std::function<void()> eventHandler_;
};

// Make shared pointer for container objects
// When container is deleted all references
// to shared objects will be deleted and hence
// objects in container will also be deleted
using EventPtr = std::shared_ptr<Event>;

class AsyncTimer {
 public:
    AsyncTimer();
    ~AsyncTimer();

    template<class F, class... Args>
    S32 createTimerEvent(S32 timeout, bool repeat, F&& f, Args&&... args);
    S32 cancelTimerEvent(S32 id);

    static AsyncTimer & getAsyncTimer();
    S32 timerLoop();
    void shutdownHandler();

    AsyncTimer(const AsyncTimer &);
    AsyncTimer(AsyncTimer &&);
    AsyncTimer & operator=(const AsyncTimer &);
    AsyncTimer & operator=(AsyncTimer &&);
 private:
    std::deque<EventPtr> eventQ_;

    std::mutex eventQMutex_;
    std::condition_variable eventQCond_;

    std::mutex emptyQMutex_;
    std::condition_variable emptyQCond_;
    bool stopThread_;
    bool fallThrough_;
};

template<class F, class... Args>
S32 AsyncTimer::createTimerEvent(S32 timeout, bool repeat, F&& f, Args&&... args) {
    TRACE();

    // Define generic funtion with any number / type of args
    // and return type, make it callable without args(func())
    // using std::bind
    auto func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
    std::function<void()> task = func;

    // Create new event object
    S32 id = std::rand();
    {
        std::unique_lock<std::mutex> lock(eventQMutex_);
        eventQ_.push_back(EventPtr(new Event(id, timeout, timeout, repeat, 0,
                                             std::chrono::system_clock::now(),
                                             task)));

        // The timer loop may already be waiting for longer
        // timeout value. If new event with lesser timeout
        // gets added to eventQ_ its necessary to notify
        // the conditional variable and fall through
        if (timeout < eventQ_.back()->timeLeft_) {
            fallThrough_ = true;
        }

        emptyQCond_.notify_one();

        return id;
    }
}

}

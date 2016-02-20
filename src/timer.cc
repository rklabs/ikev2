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

#include "timer.hh"

namespace Timer {


Event::Event(S32 id, S32 timeLeft,
             S32 timeout, bool repeat,
             S32 repeatCount,
             std::chrono::time_point<std::chrono::system_clock> startTime,
             std::function<void()> eventHandler) :
                id_(id), timeLeft_(timeLeft),
                timeout_(timeout), repeat_(repeat),
                repeatCount_(repeatCount),
                startTime_(startTime),
                eventHandler_(eventHandler) {
    TRACE();
}

AsyncTimer::AsyncTimer() : stopThread_(false), fallThrough_(false) {
    TRACE();
}

S32
AsyncTimer::timerLoop() {
    TRACE();
    while (true) {
        // Block till queue is empty
        {
            std::unique_lock<std::mutex> lock(emptyQMutex_);
            emptyQCond_.wait(lock, [this]{ return this->stopThread_ ||
                                           !this->eventQ_.empty(); });
        }

        if (stopThread_) {
            LOG(INFO, "Timer loop has been stopped");
            return 0;
        }

        if (!eventQ_.empty()) {
            // Block till least timeout value in eventQ_ is expired or
            // new event gets added to eventQ_ with lesser value
            std::unique_lock<std::mutex> lock(eventQMutex_);
            eventQCond_.wait_until(lock,
                                   std::chrono::system_clock::now() + std::chrono::milliseconds(eventQ_.back()->timeLeft_),
                                   [this] { return this->stopThread_ ||
                                            this->fallThrough_; });

            auto currTime = std::chrono::system_clock::now();

            for (auto iter = eventQ_.begin() ; iter != eventQ_.end();) {
                auto elapsedTime = \
                    std::chrono::duration_cast<std::chrono::milliseconds>(
                    currTime - (*iter)->startTime_).count();

                (*iter)->timeLeft_ -= elapsedTime;

                // If there's no time left then fire event handler
                if ((*iter)->timeLeft_ <= 0) {
                    ENQUEUE_TASK((*iter)->eventHandler_);
                    // If timer has to be repeated reset
                    // startTime_ and timeLeft_
                    if ((*iter)->repeat_) {
                        (*iter)->startTime_ = currTime;
                        (*iter)->timeLeft_ = (*iter)->timeout_;
                        (*iter)->repeatCount_++;
                        ++iter;
                        continue;
                    } else {
                        // Erase requires iterator to the element which
                        // has to be deleted. To get iterator(if not available)
                        // use std::find, std::find_if or use
                        // std::remove(remove/erase) idiom.
                        iter = eventQ_.erase(iter);
                    }
                } else {
                    (*iter)->startTime_ = currTime;
                    ++iter;
                }
            }

            // Sort once again to ensure event with shortest
            // timeLeft_ is always at back of the queue
            std::sort(eventQ_.begin(),
                      eventQ_.end(),
                      [](EventPtr(e1), EventPtr(e2)) {
                         return e1->timeLeft_ > e2->timeLeft_; });
        }
    }
}

S32
AsyncTimer::cancelTimerEvent(S32 id) {
    TRACE();

    std::unique_lock<std::mutex> lock(eventQMutex_);
    auto event = std::find_if(eventQ_.begin(),
                              eventQ_.end(),
                              [&](EventPtr(e1)) { return e1->id_ == id; });

    if (event != eventQ_.end()) {
        eventQ_.erase(event);
        return 0;
    }

    return -1;
}

AsyncTimer &
AsyncTimer::getAsyncTimer() {
    TRACE();
    static AsyncTimer asyncTimer;
    return asyncTimer;
}

void AsyncTimer::shutdownHandler() {
    TRACE();
    stopThread_ = true;
    eventQCond_.notify_all();
    emptyQCond_.notify_all();

}

AsyncTimer::~AsyncTimer() {
    TRACE();
    if (!stopThread_) {
        shutdownHandler();

    }
}

}

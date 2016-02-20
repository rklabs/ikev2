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

template<typename T>
class Queue {
 public:
    Queue();
    ~Queue();

    using Ptr = std::shared_ptr<T>;

    void addPkt(Ptr);
    bool getPkt(Ptr &);
    void shutdown();
    bool stopped();
    std::deque<Ptr> & queue();
 private:
    std::deque<Ptr> opaqueQ_;
    std::mutex queueMutex_;
    std::condition_variable queueCond_;
    bool stopped_;
};

template<typename T>
Queue<T>::Queue() : stopped_(false) {
    TRACE();
}

template<typename T>
void
Queue<T>::addPkt(Ptr elem) {
    TRACE();
    if (!stopped_) {
        std::unique_lock<std::mutex> lock(queueMutex_);
        opaqueQ_.push_back(elem);

        // Notify that packet has been added
        queueCond_.notify_one();
    }
}

template<typename T>
bool
Queue<T>::getPkt(Ptr & elem) {
    TRACE();
    {
        std::unique_lock<std::mutex> lock(queueMutex_);
        queueCond_.wait(lock, [this]{ return this->stopped_ ||
                                      !this->queue().empty(); });

        if (!opaqueQ_.empty()) {
            elem = opaqueQ_.back();
            opaqueQ_.pop_back();
            return true;
        } else {
            return false;
        }
    }
}

template<typename T>
std::deque<typename Queue<T>::Ptr> &
Queue<T>::queue() {
    TRACE();
    return opaqueQ_;
}

template<typename T>
void
Queue<T>::shutdown() {
    TRACE();
    stopped_ = true;
    queueCond_.notify_all();
}

template<typename T>
bool
Queue<T>::stopped() {
    TRACE();
    return stopped_;
}

template<typename T>
Queue<T>::~Queue() {
    TRACE();
    if (!stopped_) {
        shutdown();
    }
}

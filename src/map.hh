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

template<typename k, typename v>
class Map {
 public:
    Map();
    ~Map();

    // Create shared ptr to only 'value' in (key, value) pair.
    // Key will be trivial value which can be
    // passed around, while value is heavy weight object
    typedef std::shared_ptr<v> sharedPtrT;

    void add(k, sharedPtrT);
    bool find(k, sharedPtrT &);
    void erase(k);
    Map & map();

 private:
    std::unordered_map<k, sharedPtrT> map_;
    std::mutex mapMutex_;
    std::condition_variable mapCond_;
    bool stopped_; // XXX required?
};

template<typename k, typename v>
Map<k, v>::Map() {
    TRACE();
    stopped_ = false;
}

template<typename k, typename v>
void
Map<k, v>::add(k key, typename Map<k, v>::sharedPtrT value) {
    TRACE();

    std::unique_lock<std::mutex> lock(mapMutex_);
    map_.insert({{key, value}});

}

template<typename k, typename v>
bool
Map<k, v>::find(k key, typename Map<k, v>::sharedPtrT & value) {
    TRACE();

    std::unique_lock<std::mutex> lock(mapMutex_);
    auto iter = map_.find(key);
    if (iter != map_.end()) {
        value = iter->second;
        return true;
    }

    return false;
}

template<typename k, typename v>
void
Map<k, v>::erase(k key) {
    TRACE();
    std::unique_lock<std::mutex> lock(mapMutex_);
    map_.erase(key);
}

template<typename k, typename v>
Map<k, v> &
Map<k, v>::map() {
    TRACE();
    return map_;
}

template<typename k, typename v>
Map<k, v>::~Map() {
    TRACE();
    stopped_ = true;
}

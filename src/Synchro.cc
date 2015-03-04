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

#include "Synchro.hh"

namespace Synchro {

Notifier::Notifier(std::string notifierName) : name_(notifierName), eventFd_(0) {
    TRACE();
    LOG(INFO, "name_ %s", name_.c_str());
}

Notifier::~Notifier() {
    TRACE();
    if (eventFd_ > 0) {
        if (close(eventFd_) == -1) {
            LOG(ERROR, "Error closing event fd : %s", name_.c_str());
            perror("close");
        }
    }
    eventFd_ = 0;
}

S32 Notifier::eventFd() {
    TRACE();
    return eventFd_;
}

std::string & Notifier::name() {
    TRACE();
    return name_;
}

S32 Notifier::createNotifier(S32 initVal, S32 flags) {
    TRACE();
    eventFd_ = eventfd(initVal, flags);
    if (eventFd_ == -1) {
        LOG(ERROR, "Error creating eventfd : %s", name_.c_str());
        perror("eventfd()");
        return -1;
    }
    return eventFd_;
}

S32 Notifier::notify(S32 event) {
    TRACE();
    S32 ret = eventfd_write(eventFd_, event);
    if (ret == -1) {
        LOG(ERROR, "Error writing to eventfd : %s", name_.c_str());
        return -1;
    }
    LOG(INFO, "Notify successful : %s", name_.c_str());
    return ret;
}

S32 Notifier::readEvent(eventfd_t flag) {
    TRACE();

    eventfd_t ev;

    S32 ret = eventfd_read(eventFd_, &ev);
    if (ret == -1) {
        LOG(ERROR, "Error reading eventfd : %s", name_.c_str());
        return -1;
    }

    // If particular event has been read return success
    if (ev == flag) {
        LOG(INFO, "Read %" PRId64 " from eventfd %s", ev, name_.c_str());
        // Return success
        return 1;
    } else {
        LOG(ERROR, "Invalid read %" PRId64 ", %" PRId64 ", %s", ev, flag, name_.c_str());
        return 0;
    }

    return 0;
}

}  // namespace Synchro

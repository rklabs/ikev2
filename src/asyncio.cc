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

#include "asyncio.hh"

namespace ASIO {

AsyncIOHandler::AsyncIOHandler(S32 noOfEvents,
                               std::string pollerName) : epollFd_(0),
                                                         maxEvents_(noOfEvents),
                                                         stopPoller_(false),
                                                         name_(pollerName) {
    TRACE();

}

S32 AsyncIOHandler::createPoller() {
    TRACE();

    // Create poller fd
    epollFd_ = epoll_create1(0);
    if (epollFd_ == -1) {
        LOG(ERROR, "epoll_create() : %s", name_.c_str());
        perror("epoll_create");
        return -1;
    }

    LOG(INFO, "Successfully created epoll fd : %s", name_.c_str());
    return 0;
}

S32 AsyncIOHandler::addFd(S32 fd) {
    TRACE();
    auto inputEvent_ = epollEventPtr(new struct epoll_event);

    // Zeroize struct epoll_event to suppress valgrind warning
    memset(inputEvent_.get(), 0, sizeof(struct epoll_event));

    // Poll for input events on inotify fd
    inputEvent_->events = EPOLLIN;
    inputEvent_->data.fd = fd;

    watchedEvents_.push_back(inputEvent_);

    // Need to pass raw pointer to system calls
    if (epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, inputEvent_.get()) == -1) {
        LOG(ERROR, "epoll_ctl() : %s", name_.c_str());
        perror("epoll_ctl() : ");
        return -1;
    }

    LOG(INFO, "Successfully added fd : %s", name_.c_str());
    return 0;
}

S32 AsyncIOHandler::modifyFd(S32 fd, S32 op) {
    TRACE();

    // Find event with matching fd
    auto event = std::find_if(watchedEvents_.begin(),
                              watchedEvents_.end(),
                              [&](epollEventPtr(e)) {
                                  return e->data.fd == fd; });

    if (event != watchedEvents_.end()) {
        if (epoll_ctl(epollFd_, op, (*event)->data.fd, (*event).get()) == -1) {
            LOG(ERROR, "epoll_ctl() modify failed : %s", name_.c_str());
            perror("epoll_ctl() modify : ");
            return -1;
        }
        LOG(INFO, "Successfully modified fd %d : %s", fd, name_.c_str());
        return 0;
    } else {
        LOG(ERROR, "Failed to modify fd %d : %s", fd, name_.c_str());
        return 1;
    }
}

S32 AsyncIOHandler::removeFd(S32 fd) {
    TRACE();

    // std::find_if returns iterator to the element matching
    // predicate else returns end()
    auto event = std::find_if(watchedEvents_.begin(),
                              watchedEvents_.end(),
                              [&](epollEventPtr(e)) {
                                  return e->data.fd == fd; });

    if (event != watchedEvents_.end()) {
        watchedEvents_.erase(event, watchedEvents_.end());
        LOG(INFO, "Successfully removed fd %d : %s", fd, name_.c_str());
        return 0;
    } else {
        LOG(ERROR, "fd not found %d : %s", fd, name_.c_str());
        return -1;
    }
}

S32 AsyncIOHandler::watchFds() {
    TRACE();
    const S32 max = maxEvents_;
    struct epoll_event outputEvents_[max];

    memset(&outputEvents_, 0, maxEvents_ * sizeof(struct epoll_event));

    LOG(INFO, "Watching %s", name_.c_str());

    if (!stopPoller_) {
        // Block here waiting for events
        S32 nfds = epoll_wait(epollFd_, outputEvents_,
                              maxEvents_, EPOLL_BLOCK_FD);

        if (nfds == 0) {
            LOG(INFO, "Waiting for event : %s", name_.c_str());
            return 0;
        } else if (nfds == -1) {
            LOG(ERROR, "epoll_pwait() : %s", name_.c_str());
            perror("epoll_pwait");
            return -1;
        }

        LOG(INFO, "nfds %d : %s", nfds, name_.c_str());

        for (S32 idx = 0; idx < nfds; ++idx) {
            if ((outputEvents_[idx].events & EPOLLERR) ||
                (outputEvents_[idx].events & EPOLLHUP) ||
                (!(outputEvents_[idx].events & EPOLLIN))) {

                LOG(INFO, "epoll error : %s", name_.c_str());
                close(outputEvents_[idx].data.fd);
                return -1;
            }

            for (auto & iter : watchedEvents_) {
                // Read event on watched fd
                if (outputEvents_[idx].data.fd == iter->data.fd) {
                    LOG(INFO, "Event triggered on fd %d : %s", iter->data.fd, name_.c_str());
                    return iter->data.fd;
                }
            }
        }
    } else {
        LOG(INFO, "Poller %s has been stopped", name_.c_str());
        return 0;
    }

    return 0;
}

void AsyncIOHandler::shutdownHandler() {
    // XXX mutex if multiple threads use same asyncio obj
    stopPoller_ = true;
}

AsyncIOHandler::~AsyncIOHandler() {
    TRACE();
    if (epollFd_ > 0) {
        if (close(epollFd_) == -1) {
            LOG(ERROR, "Error closing epoll fd : %s", name_.c_str());
            perror("close");
        }
    }
}

}  // namespace ASIO

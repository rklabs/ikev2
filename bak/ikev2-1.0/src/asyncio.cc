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

AsyncIOHandler::AsyncIOHandler(int num, std::string n) : epollFd_(0),
                                                         maxEvents_(num),
                                                         stop_(false),
                                                         name_(n) {
    TRACE();

}

int AsyncIOHandler::create() {
    TRACE();

    // Create poller fd
    epollFd_ = epoll_create(maxEvents_);
    if (epollFd_ == -1) {
        LOG(ERROR, "epoll_create() : %s", name_.c_str());
        perror("epoll_create");
        return -1;
    }

    LOG(INFO, "Successfully created epoll fd : %s", name_.c_str());
    return 0;
}

int AsyncIOHandler::addFd(int fd) {
    TRACE();
    struct epoll_event inputEvent_;

    memset(&inputEvent_, 0, sizeof(struct epoll_event));

    // Poll for input events on inotify fd
    inputEvent_.events = EPOLLIN;
    inputEvent_.data.fd = fd;

    watchedFds_.push_back(fd);

    if (epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &inputEvent_) == -1) {
        LOG(ERROR, "epoll_ctl() : %s", name_.c_str());
        perror("epoll_ctl() : ");
        return -1;
    }

    LOG(INFO, "Successfully added fd : %s", name_.c_str());
    return 0;
}

int AsyncIOHandler::modifyFd(int fd) {
    TRACE();

    LOG(INFO, "Successfully modified fd : %s", name_.c_str());

    return 0;
}

int AsyncIOHandler::removeFd(int fd) {
    TRACE();

    watchedFds_.erase( std::remove( std::begin(watchedFds_), std::end(watchedFds_), fd ), std::end(watchedFds_) );

    LOG(INFO, "Successfully removed fd : %s", name_.c_str());

    return 0;
}

int AsyncIOHandler::watch() {
    TRACE();
    const int max = maxEvents_;
    struct epoll_event outputEvents_[max];

    memset(&outputEvents_, 0, maxEvents_ * sizeof(struct epoll_event));

    LOG(INFO, "Watching %s", name_.c_str());

    if (!stop_) {
        int nfds = epoll_wait(epollFd_, outputEvents_,
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

        for (int n = 0; n < nfds; ++n) {
            if ((outputEvents_[n].events & EPOLLERR) ||
                (outputEvents_[n].events & EPOLLHUP) ||
                (!(outputEvents_[n].events & EPOLLIN))) {

                // An error has occured on this fd, or the fd is not
                // ready for reading (why were we notified then?)
                LOG(INFO, "epoll error : %s", name_.c_str());
                close(outputEvents_[n].data.fd);
                return 0;
            }

            for (auto & iter : watchedFds_) {
                // Read event on watched fd
                if (outputEvents_[n].data.fd == iter) {
                    LOG(INFO, "Event triggered on fd %d : %s", iter, name_.c_str());
                    return iter;
                }
            }
        }
    } else {
        LOG(INFO, "Poller has been stopped");
        return 0;
    }

    return 0;
}

void AsyncIOHandler::stopIs(bool s) {
    stop_ = s;
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

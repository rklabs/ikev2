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

#include "ikev2config.hh"

namespace IKEv2 {

Config::Config() : inotifyFd_(0), inotifyWd_(0), eventFd_(0),
                   stopThread_(false), eventNotifier_("cfgNotifier"),
                   asioHdl_(CFG_MAX_EVENTS, "cfgPoller") {
    TRACE();
}

Synchro::Notifier &
Config::eventNotifier() {
    return eventNotifier_;
}

void
Config::shutdown() {
    TRACE();
    LOG(INFO, "Stopping ConfFileWatcher thread");

    if (!stopThread_) {
        stopThread_ = true;
    }

    LOG(INFO, "Cleaning up inotify");
    if (inotifyFd_ > 0 && inotifyWd_ > 0) {
        if (inotify_rm_watch(inotifyFd_, inotifyWd_) == -1) {
            LOG(ERROR, "Error calling inotify_rm_watch");
            perror("inotify_rm_watch");
        }

        if (close(inotifyFd_) == -1) {
            LOG(ERROR, "Error calling close on inotify fd");
            perror("close() : fd");
        }
    }
    LOG(INFO, "inotify fd has been closed successfully");

    inotifyFd_ = inotifyWd_ = 0;
    LOG(INFO, "epoll fd has been closed successfully");
}

Config::~Config() {
    TRACE();
    if (stopThread_) {
        shutdown();
    }
}

int
Config::confFilePresent() {
    TRACE();
    return access(IKEV2_CONF_FILE, F_OK) != -1;
}

int
Config::confFileWatcher() {
    TRACE();
    int buffLength;
    char *ptr;
    char buffer[INOTIFY_BUFFER_LEN];
    //__attribute__((aligned(__alignof__(EVENT_SIZE))));
    struct inotify_event *event;

    if (!confFilePresent()) {
        LOGT("%s does not exist", IKEV2_CONF_FILE);
        return -1;
    }

    // Create inotify descriptor
    inotifyFd_ = inotify_init();

    Utils::setFdNonBlocking(inotifyFd_ );

    if (inotifyFd_  == -1) {
        LOG(ERROR, "inotify_init()");
        perror("inotify_init()");
        return -1;
    }

    // Watch config file
    inotifyWd_ = inotify_add_watch(inotifyFd_, IKEV2_CONF_FILE,
                                   IN_ALL_EVENTS);
    if (inotifyWd_ == -1) {
        LOG(ERROR, "inotify_add_watch()");
        perror("inotify_add_watch()");
        return -1;
    }

    if (asioHdl_.create() == -1) {
        return -1;
    }

    if (asioHdl_.addFd(inotifyFd_) == -1) {
        return -1;
    }

    // Create notifier event for stopping thread
    // Main thread will notify if thread has be be
    // cleaned up in case of success or failure
    eventFd_ = eventNotifier_.create(0, 0);
    if (eventFd_ == -1) {
        LOG(ERROR, "Failed to create eventfd");
        return -1;
    }

    if (asioHdl_.addFd(eventFd_) == -1) {
        return -1;
    }

    // Main thread loop
    while (true) {
         if (!stopThread_) {
            // Block here waiting for event
            int polledFd = asioHdl_.watch();
            if (polledFd == inotifyFd_) {
                // Read event
                buffLength = read(inotifyFd_, buffer, INOTIFY_BUFFER_LEN);

                if (buffLength == -1) {
                    LOG(ERROR, "read()");
                    perror("read()");
                    return -1;  // XXX return or continue?
                }

                LOG(INFO, "Read %d bytes from inotify fd", buffLength);

                for (ptr = buffer ; ptr < buffer + buffLength ;) {
                    event = (struct inotify_event *)ptr;
                    LOG(INFO, "event->mask %x event->len %d",
                            event->mask, event->len);

                    // File create event
                    if (event->mask & IN_CREATE) {
                        LOG(INFO, "%s was created", IKEV2_CONF_FILE);
                    // File delete event
                    } else if (event->mask & IN_DELETE_SELF) {
                        LOG(INFO, "%s has been deleted", IKEV2_CONF_FILE);
                        LOGT("%s has been deleted", IKEV2_CONF_FILE);
                        LOGT("IKEv2 will shutdown now");

                        return -1;
                    // File modified event
                    } else if (event->mask & IN_MODIFY) {
                        LOG(INFO, "%s was modified", IKEV2_CONF_FILE);
                        // XXX Need to reload daemon
                    }
                    ptr += EVENT_SIZE + event->len;
                }  // end of for (p = buffer)...
            } else if (polledFd == eventFd_) {
                if (eventNotifier_.wait(STOP_CFG_THREAD)) {
                    LOG(INFO, "Cfg thread received stop event");
                    stopThread_ = true;
                }
            } else {
                LOG(ERROR, "Unknown fd polled %d", polledFd);
            }
        } else {
            LOG(INFO, "Cfg thread stopped" );
            return 0;
        }
    }  // end of while (true)

    return 0;
}

}  // namespace IKEv2

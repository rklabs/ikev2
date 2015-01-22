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

#include "utils.hh"

namespace Utils {

int setResourceLimit() {
    TRACE();
    // core dumps may be disallowed by parent of this process
    struct rlimit core_limits;
    core_limits.rlim_cur = core_limits.rlim_max = RLIM_INFINITY;
    if (setrlimit(RLIMIT_CORE, &core_limits) == -1) {
        LOG(ERROR, "setrlimit: %s", strerror(errno));
        perror("setrlimit");
        return -1;
    }

    LOG(INFO, "ulimit -c unlimited success!");

    return 0;
}

int
setFdNonBlocking(int fd) {
    int flags, ret;

    // Save flags
    flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        perror ("fcntl");
        return -1;
    }

    // Make descriptor non-blocking
    flags |= O_NONBLOCK;
    ret = fcntl(fd, F_SETFL, flags);
    if (ret == -1) {
        perror ("fcntl");
        return -1;
    }

    return 0;
}

}

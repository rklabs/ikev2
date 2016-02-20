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

#include <unistd.h>
#include <inttypes.h>
#include <sys/eventfd.h>

#include "basictypes.hh"
#include "logging.hh"

namespace Synchro {

class Notifier {
 public:
    Notifier(std::string notifierName);
    ~Notifier();
    S32 eventFd();
    std::string & name();
    S32 createNotifier(S32 initVal, S32 flags);
    S32 notify(S32 event);
    S32 readEvent(eventfd_t flag);
 private:
    std::string name_;
    S32 eventFd_;

};

}  // namespace Synchro

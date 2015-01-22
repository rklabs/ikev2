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

#include "logging.hh"
#include "ikev2payload.hh"

namespace IKEv2 {
class Packet {
 public:
    typedef struct ikev2Header_s {
        uint32_t initiatorSpi;
        uint32_t responderSpi;
        uint8_t nextPayload;
        uint8_t version;  // Major + Minor version
        uint16_t xchgType;
        uint8_t flags;
        uint32_t msgId;
        uint32_t length;
    }ikev2Header;

    Packet();
    ~Packet();
    int create();
    int destroy();
    int clone();
};
}  // namespace IKEv2

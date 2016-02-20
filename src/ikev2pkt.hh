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
        U32 initiatorSpi;
        U32 responderSpi;
        U8 nextPayload;
        U8 version;  // Major + Minor version
        U16 xchgType;
        U8 flags;
        U32 msgId;
        U32 length;
    }ikev2Header;

    Packet();
    ~Packet();
    S32 create();
    S32 destroy();
    S32 clone();
};
}  // namespace IKEv2

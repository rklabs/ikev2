/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
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

#ifndef _IKEV2_SRC_NETWORK_H_
#define _IKEV2_SRC_NETWORK_H_

#include <netdb.h>   // getaddrinfo()
#include <string.h>  // memset()
#include <unistd.h>  // close()
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include "src/logging.hh"

#define IKEV2_UDP_DEST_PORT "500"

namespace ikev2 {
namespace network {
    class udpSocket {
     public:
         udpSocket();
         ~udpSocket();
         int sendPacket();
         int receivePacket();
     private:
        int sockfd, new_fd;
        std::string peerAddress;
        std::string sourceInterface;
        std::string sourceAddress;
    };
}  // namespace network
}  // namespace ikev2


#endif  // _IKEV2_SRC_NETWORK_H_

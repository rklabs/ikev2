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

#include "src/network.hh"
#include "src/logging.hh"

namespace ikev2 {
namespace network {
    udpSocket::udpSocket() {
        TRACE();
        struct addrinfo intfHint, *intfInfo, *iter;
        struct sockaddr_in *ipv4 = NULL;
        char ipAddr[INET6_ADDRSTRLEN];
        std::string ipVersion;
        int ret = 0;

        // Find first ip address to be used as source interface
        memset(&intfHint, 0, sizeof(intfHint));
        intfHint.ai_family = AF_UNSPEC;
        intfHint.ai_socktype = SOCK_DGRAM;
        intfHint.ai_flags = AI_PASSIVE;

        ret = getaddrinfo(NULL, IKEV2_UDP_DEST_PORT, &intfHint, &intfInfo);
        if (ret != 0) {
            LOG(LOG::ERROR, "getaddrinfo: %s", gai_strerror(ret));
            exit(EXIT_FAILURE);
        }

        // Loop through result and use first interface for sending packet
        for (iter = intfInfo; iter != NULL; iter = iter->ai_next) {
            if ((sockfd = socket(iter->ai_family, \
                                 iter->ai_socktype, \
                                 iter->ai_protocol)) == -1) {
                LOG(LOG::ERROR, "socket() error");
                continue;
            }
            if (iter->ai_family == AF_INET) {
                ipv4 = (struct sockaddr_in *)iter->ai_addr;
                ipVersion = "IPv4";
            }
            break;
        }

        // If no address is found exit
        if (iter == NULL) {
            LOG(LOG::ERROR, "No suitable ip address found!");
            exit(EXIT_FAILURE);
        }

        // Convert ip address to string
        inet_ntop(iter->ai_family, &(ipv4->sin_addr), ipAddr, sizeof(ipAddr));
        LOG(LOG::INFO, "Using %s: %s", ipVersion.c_str(), ipAddr);
    }

    int udpSocket::sendPacket() {
        struct sockaddr_in peer;
        struct msghdr msg;
        struct iovec iov;

        return 0;
    }

    udpSocket::~udpSocket() {
        TRACE();
        close(sockfd);
    }
}  // namespace network
}  // namespace ikev2

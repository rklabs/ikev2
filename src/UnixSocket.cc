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

#include "UnixSocket.hh"

namespace Socket {

U32 UnixSocket::createSocket(IpAddress addr, U32 port, AddressFamily addrFamily) {
    S32 ret = 0;
    S32 reUseAddr = 1;
    char ipAddr[INET_ADDRSTRLEN];
    struct addrinfo intfHint;
    struct addrinfo* intfInfo;
    struct addrinfo* iter;

    // Do this in caller
    assert(isIpAddress(peerAddress_));
    assert(std::stoi(peerPort_) > 0);

    memset(&intfHint, 0, sizeof(intfHint));

    if (addrFamily == AddressFamily::ANY) {
        intfHint.ai_family = AF_UNSPEC;
    } else if (addrFamily == AddressFamily::IPv4) {
        intfHint.ai_family = AF_INET;
    } else if (addrFamily == AddressFamily::IPv6) {
        intfHint.ai_family = AF_INET6;
    } else {
        assert(false && "Invalid address family");
    }

    intfHint.ai_socktype = SOCK_DGRAM;
    intfHint.ai_flags = AI_PASSIVE;

    if (addr.size() == 0) {
        ret = getaddrinfo(nullptr, port, &intfHint, &intfInfo);
    } else {
        ret = getaddrinfo(addr.c_str(), port, &intfHint, &intfInfo);
    }
    if (ret != 0) {
        LOGT("IPv4: Failed getting address info");
        LOG(ERROR, "IPv4: getaddrinfo: %s", gai_strerror(ret));
        perror("getaddrinfo()");
        return -1;
    }

    // Loop through interfaces and use first interface for sending packet
    for (iter = intfInfo; iter != nullptr; iter = iter->ai_next) {
        // If AF is IPV4
        if (iter->ai_family == AF_INET) {
            sockfd_ = socket(iter->ai_family, iter->ai_socktype, iter->ai_protocol);
            if (sockfd_ == -1) {
                LOG(ERROR, "IPv4: socket() error");
                perror("IPV4 socket()");
                continue;
            }

            // Make socket reusable
            if (setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR,
                           &reUseAddr, sizeof(reUseAddr)) == -1) {
                LOG(ERROR, "IPv4: setsockopt failed to set SO_REUSEADDR");
                perror("setsockopt");
                return -1;
            }

            struct sockaddr_in *ipv4 = (struct sockaddr_in *)iter->ai_addr;
            ipVersionIs("IPv4");

            // If bind fails close socket and try next interface
            if (bind(sockfd_, (struct sockaddr *)ipv4, sizeof(*ipv4)) == -1) {
                perror("IPv4: bind failed");
                close(sockfd_);
                continue;
            }

            LOG(INFO, "IPv4: Successfully bound to port %s", IKEV2_UDP_PORT);
            LOG(INFO, "IPv4: Socket is ready to receive / send data");

            // Convert ip address to string
            inet_ntop(iter->ai_family, &(ipv4->sin_addr), ipAddr, sizeof(ipAddr));
            LOG(INFO, "IPv4: Using %s source address %s:%d", ipVersion_.c_str(), ipAddr,
                                                       htons(ipv4->sin_port));
            sourceAddressIs(ipAddr);
            break;
        }
    }

    // If no address was found exit
    if (iter == nullptr) {
        LOGT("IPv4: No address was found on device. Exiting.");
        return -1;
    } else {
        freeaddrinfo(intfInfo);
    }

    return sockfd_;

}

} // namespace Socket

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

#include "network.hh"
#include "logging.hh"

namespace Network {

// Start of class NetworkEndpoint

NetworkEndpoint::NetworkEndpoint() {
    TRACE();
}

NetworkEndpoint::~NetworkEndpoint() {
    TRACE();
}

// Start of class NetworkEndpoint

// Start of class UdpEndpoint
UdpEndpoint::UdpEndpoint(const ipAddress & destination,
                         const port  & destPort,
                         AddressFamily addrFamily) :
                            stopThread_(false),
                            eventFd_(0),
                            sockfd_(0),
                            peerAddress_(destination),
                            peerPort_(destPort),
                            addrFamily_(addrFamily),
                            eventNotifier_("nwNotifier") {
    TRACE();
}

UdpEndpoint::~UdpEndpoint() {
    TRACE();
}

inline void
UdpEndpoint::sourceAddressIs(const ipAddress & addr) {
    TRACE();
    sourceAddress_ = addr;
}

inline void
UdpEndpoint::peerAddressIs(const ipAddress & addr) {
    TRACE();
    peerAddress_ = addr;
}

inline void
UdpEndpoint::sourceInterfaceIs(const interface & intf) {
    TRACE();
    sourceInterface_ = intf;
}

bool
UdpEndpoint::isIpAddress(const ipAddress & ipAddr) {
    TRACE();
    int result = 0;

    if (addrFamily_ == AddressFamily::IPV4) {
        struct sockaddr_in sa;
        result = inet_pton(AF_INET, ipAddr.c_str(), &(sa.sin_addr));
    } else if (addrFamily_ == AddressFamily::IPV6) {
        struct sockaddr_in6 sa;
        result = inet_pton(AF_INET6, ipAddr.c_str(), &(sa.sin6_addr));
    } else {
        assert(false && "Invalid address family!");
    }

    return result != 0;
}

ipAddress
UdpEndpoint::peerAddress() {
    TRACE();
    return peerAddress_;
}

interface
UdpEndpoint::sourceInterface() {
    TRACE();
    return sourceInterface_;
}

ipAddress
UdpEndpoint::sourceAddress() {
    TRACE();
    return sourceAddress_;
}

inline void
UdpEndpoint::ipVersionIs(const ipVersion & version) {
    ipVersion_ = version;
}

Synchro::Notifier &
UdpEndpoint::eventNotifier() {
    return eventNotifier_;
}

std::string UdpEndpoint::sinAddrToStr(void * peer) {
    if (addrFamily_ == AddressFamily::IPV4) {
        char straddr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, peer, straddr, sizeof(straddr));
        return straddr;
    } else if (addrFamily_ == AddressFamily::IPV6) {
        char straddr[INET6_ADDRSTRLEN];
        inet_ntop(AF_INET6, peer, straddr, sizeof(straddr));
        return straddr;
    }

    return nullptr;
}

// End of class UdpEndpoint

// Start of class UdpEndpoint4

UdpEndpoint4::UdpEndpoint4(const ipAddress & destination,
                           const port & destPort) :
                           UdpEndpoint::UdpEndpoint(destination,
                                                    destPort,
                                                    AddressFamily::IPV4) {
    TRACE();
    //sendQ_ = Processor::CreatePacketQueue<std::string, PeerInfo4>();
    //rcvQueue_ = Processor::CreatePacketQueue<std::string, PeerInfo4>();
}

UdpEndpoint4::UdpEndpoint4(const UdpEndpoint4 & other) : UdpEndpoint::UdpEndpoint(other.peerAddress_,
                         other.peerPort_,
                            AddressFamily::IPV4)  {
    TRACE();
}

int
UdpEndpoint4::initUdpEndpoint() {
    TRACE();

    int ret = 0;
    int reUseAddr = 1;
    char ipAddr[INET_ADDRSTRLEN];
    struct addrinfo intfHint, *intfInfo, *iter;

    assert(isIpAddress(peerAddress_));
    assert(std::stoi(peerPort_) > 0);

    // Find first ip address to be used as source interface
    memset(&intfHint, 0, sizeof(intfHint));
    intfHint.ai_family = AF_UNSPEC;
    intfHint.ai_socktype = SOCK_DGRAM;
    intfHint.ai_flags = AI_PASSIVE;

    // Get interfaces on this device
    ret = getaddrinfo(nullptr, IKEV2_UDP_PORT, &intfHint, &intfInfo);
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

            if (bind(sockfd_, (struct sockaddr *)ipv4, sizeof(*ipv4)) == -1) {
                perror("IPv4: bind failed");
                return -1;
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

    return 0;
}

int
UdpEndpoint4::send() {
    TRACE();
    //struct sockaddr_in peer;
    //struct msghdr msg;
    //struct iovec iov;
    //sendmsg

    LOG(INFO, "IPv4: Send thread created successfully");

    // deque packets and send
    while (true) {
        // Wait for sendQueue to have some work
        {
            std::unique_lock<std::mutex> lock(sendQeueueMutex);
            condition.wait(lock, [this](){ return stopThread_ ||
                                           !sendQ_.queue().empty(); } );

            if (stopThread_) {
                return 0;
            } else if (sendQ_.queue().empty()) {
                continue;
            }
        }

        // Iterate over send queue and handle data
        for (auto iter = sendQ_.queue().begin() ;
             iter != sendQ_.queue().end() ;) {
            int ret = sendto(sockfd_, &iter->second.buffer, iter->second.bufferLen,
                             0, (struct sockaddr *)&iter->second.peer,
                             sizeof(iter->second.peer));
            if (ret == -1) {
                LOG(ERROR, "IPv4: Error in sendto");
                perror("sendto");
                return -1;
            }

            // Remove packet from the queue
            sendQ_.queue().erase(iter++);
        }
        LOG(INFO, "IPv4: rcvQ.size() %d", rcvQ_.queue().size());
        LOG(INFO, "IPv4: sendQ.size() %d", sendQ_.queue().size());
        LOG(INFO, "IPv4: rcvQ.empty() %d", rcvQ_.queue().empty());
        LOG(INFO, "IPv4: sendQ.empty() %d", sendQ_.queue().empty());
    }  // end of while (true)
    return 0;
}

void
UdpEndpoint4::handleConnection() {
    TRACE();
    // Iterate over receive queue and notify
    for (auto iter = rcvQ_.queue().begin() ;
         iter != rcvQ_.queue().end() ;) {
        PeerInfo4 peerInfo;
        peerInfo.peer = iter->second.peer;
        peerInfo.bufferLen = iter->second.bufferLen;
        memcpy(peerInfo.buffer, iter->second.buffer, iter->second.bufferLen);

        ipAddress ipAddr = sinAddrToStr((void*)&peerInfo.peer.sin_addr);
        port prt = std::to_string(ntohs(peerInfo.peer.sin_port));

        LOG(INFO, "IPv4: Received packet from %s:%s", ipAddr.c_str(), prt.c_str());

        sendQ_.enqueue(ipAddr + ":" + prt, peerInfo);

        condition.notify_all();

        rcvQ_.queue().erase(iter++);
    }
};

int
UdpEndpoint4::receive() {
    TRACE();
    int bytes;
    int peerLen = sizeof(struct sockaddr_in);
    char buffer[BUFFLEN];
    struct sockaddr_in peer;
    ASIO::AsyncIOHandler asioHdl(NW_MAX_EVENTS, "nwPoller4");

    // Make socket non-blocking
    if (Utils::setFdNonBlocking(sockfd_) == -1) {
        LOG(ERROR, "IPv4: Failed to make server socket non-blocking");
    }

    // Create poller object
    if (asioHdl.create() == -1) {
        return -1;
    }

    // Add nw socket to poller object
    if (asioHdl.addFd(sockfd_) == -1) {
        return -1;
    }

    // Create notifier event for stopping thread
    // Main thread will notify if thread has be be
    // cleaned up in case of success or failure
    eventFd_ = eventNotifier_.create(0, EFD_SEMAPHORE);
    if (eventFd_ == -1) {
        LOG(ERROR, "IPv4: Failed to create eventfd");
        return -1;
    }

    // Add event fd to poller object
    if (asioHdl.addFd(eventFd_) == -1) {
        return -1;
    }

    // Main thread loop
    while (true) {
        if (!stopThread_) {
            // Block here waiting for event on poller
            int polledFd = asioHdl.watch();
            if (polledFd == sockfd_) {
                LOG(INFO, "IPv4: UDP socket has become available");
                bytes = recvfrom(sockfd_, buffer, BUFFLEN, 0,
                                 (struct sockaddr *)&peer,
                                 (socklen_t *)&peerLen);

                if (bytes == -1) {
                    LOG(ERROR, "IPv4: recvfrom() failed in receive()");
                    perror("recvfrom");
                    return -1;
                }

                buffer[bytes] = '\0';

                PeerInfo4 peerInfo;
                peerInfo.peer = peer;
                peerInfo.bufferLen = bytes;
                memcpy(peerInfo.buffer, buffer, peerInfo.bufferLen);

                ipAddress ipAddr = sinAddrToStr((void*)&peerInfo.peer.sin_addr);
                port prt = std::to_string(ntohs(peerInfo.peer.sin_port));

                LOG(INFO, "IPv4: Received packet from %s:%s", ipAddr.c_str(), prt.c_str());
                LOGT("IPv4: Client sent data : %s:%s", ipAddr.c_str(), prt.c_str());

                rcvQ_.enqueue(ipAddr + ":" + prt, peerInfo);

                // handle connection
                handleConnection();

                LOG(INFO, "IPv4: Data: %s", buffer);
            } else if (polledFd == eventFd_) {
                if (eventNotifier_.wait(STOP_NW_THREAD)) {
                    LOG(INFO, "IPv4: Nw thread received stop event");
                    stopThread_ = true;
                    // Notify all threads to stop executing
                    condition.notify_all();
                }
            } else {
                LOG(ERROR, "IPv4: Unknown fd polled %d", polledFd);
            }
        } else {
            LOG(INFO, "IPv4: Nw thread stopped" );
            return 0;
        }
    }  // end of while (true)

    return 0;
}

UdpEndpoint4::~UdpEndpoint4() {
    TRACE();

    if (!stopThread_) {
        stopThread_ = true;
        condition.notify_all();
    }

    LOG(INFO, "IPv4: Closing nw socket");

    if (sockfd_ > 0) {
        if (close(sockfd_) == -1) {
            LOG(ERROR, "IPv4: Error closing socket");
            perror("close()");
        }
    }
    sockfd_ = 0;
}

// End of class UdpEndpoint4

// Start of UdpEndpoint6
UdpEndpoint6::UdpEndpoint6(const ipAddress & destination,
                           const port & destPort) :
                           UdpEndpoint::UdpEndpoint(destination,
                                                    destPort,
                                                    AddressFamily::IPV6) {
    TRACE();
    //sendQ_ = Processor::CreatePacketQueue<std::string, PeerInfo6>();
    //rcvQueue_ = Processor::CreatePacketQueue<std::string, PeerInfo6>();
}

UdpEndpoint6::UdpEndpoint6(const UdpEndpoint6 & other) :   UdpEndpoint::UdpEndpoint(other.peerAddress_,
                         other.peerPort_,
                            AddressFamily::IPV6) {
    TRACE();
}


int
UdpEndpoint6::initUdpEndpoint() {
    TRACE();

    int ret = 0;
    int reUseAddr = 1;
    char ipAddr[INET6_ADDRSTRLEN];
    struct addrinfo intfHint;
    struct addrinfo *intfInfo;
    struct addrinfo *iter;

    assert(isIpAddress(peerAddress_));
    assert(std::stoi(peerPort_) > 0);

    // Find first ip address to be used as source interface
    memset(&intfHint, 0, sizeof(intfHint));
    intfHint.ai_family = AF_UNSPEC;
    intfHint.ai_socktype = SOCK_DGRAM;
    intfHint.ai_flags = AI_PASSIVE;

    // Get interfaces on this device
    ret = getaddrinfo(nullptr, IKEV2_UDP_PORT, &intfHint, &intfInfo);
    if (ret != 0) {
        LOGT("IPv6: Failed getting address info");
        LOG(ERROR, "IPv6: getaddrinfo: %s", gai_strerror(ret));
        perror("getaddrinfo()");
        return -1;
    }

    // Loop through interfaces and use first interface for sending packet
    for (iter = intfInfo; iter != nullptr; iter = iter->ai_next) {
        if (iter->ai_family == AF_INET6) {
            sockfd_ = socket(iter->ai_family, iter->ai_socktype, iter->ai_protocol);
            if (sockfd_ == -1) {
                LOG(ERROR, "IPv6: socket() error");
                perror("IPV6 socket()");
                continue;
            }

            // Make socket reusable
            if (setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR,
                &reUseAddr, sizeof(reUseAddr)) == -1) {
                LOG(ERROR, "IPv6: setsockopt failed to set SO_REUSEADDR");
                perror("setsockopt");
                return -1;
            }

            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)iter->ai_addr;
            ipVersionIs("IPv6");

            if (bind(sockfd_, (struct sockaddr *)ipv6, sizeof(*ipv6)) == -1) {
                perror("IPv6: bind failed");
                return -1;
            }

            LOG(INFO, "IPv6: Successfully bound to port %s", IKEV2_UDP_PORT);
            LOG(INFO, "IPv6: Socket is ready to receive / send data");

            // Convert ip address to string
            inet_ntop(iter->ai_family, &(ipv6->sin6_addr), ipAddr, sizeof(ipAddr));
            LOG(INFO, "IPv6: Using %s source address %s:%d", ipVersion_.c_str(),
                                                             ipAddr,
                                                             htons(ipv6->sin6_port));
            sourceAddressIs(ipAddr);
            break;
        }
    }

    // If no address was found exit
    if (iter == nullptr) {
        LOGT("IPv6: No address was found on device. Exiting.");
        return -1;
    } else {
        freeaddrinfo(intfInfo);
    }

    return 0;
}

int
UdpEndpoint6::send() {
    TRACE();
    //struct sockaddr_in peer;
    //struct msghdr msg;
    //struct iovec iov;
    //sendmsg

    LOG(INFO, "IPv6: Send thread created successfully");

    // deque packets and send
    while (true) {
        // Wait for sendQueue to have some work
        {
            std::unique_lock<std::mutex> lock(sendQeueueMutex);
            condition.wait(lock, [this](){ return stopThread_ ||
                                           !sendQ_.queue().empty(); } );

            if (stopThread_) {
                return 0;
            } else if (sendQ_.queue().empty()) {
                continue;
            }
        }

        // Iterate over send queue and handle data
        for (auto iter = sendQ_.queue().begin() ;
             iter != sendQ_.queue().end() ;) {
            int ret = sendto(sockfd_, &iter->second.buffer, iter->second.bufferLen,
                             0, (struct sockaddr *)&iter->second.peer,
                             sizeof(iter->second.peer));
            if (ret == -1) {
                LOG(ERROR, "IPv6: Error in sendto");
                perror("sendto");
                return -1;
            }

            // Remove packet from the queue
            sendQ_.queue().erase(iter++);
        }
        LOG(INFO, "IPv6: rcvQueue_.queue().size() %d", rcvQ_.queue().size());
        LOG(INFO, "IPv6: sendQ_.queue().size() %d", sendQ_.queue().size());
        LOG(INFO, "IPv6: rcvQueue_.queue().empty() %d", rcvQ_.queue().empty());
        LOG(INFO, "IPv6: sendQ_.queue().empty() %d", sendQ_.queue().empty());
    }  // end of while (true)
    return 0;
}

void
UdpEndpoint6::handleConnection() {
    TRACE();

    // Iterate over receive queue and notify
    for (auto iter = rcvQ_.queue().begin() ;
         iter != rcvQ_.queue().end() ;) {
        PeerInfo6 peerInfo;
        peerInfo.peer = iter->second.peer;
        peerInfo.bufferLen = iter->second.bufferLen;
        memcpy(peerInfo.buffer, iter->second.buffer, iter->second.bufferLen);

        ipAddress ipAddr = sinAddrToStr((void*)&peerInfo.peer.sin6_addr);
        port prt = std::to_string(ntohs(peerInfo.peer.sin6_port));
        LOG(INFO, "IPv6: Received packet from %s:%s", ipAddr.c_str(), prt.c_str());

        sendQ_.enqueue(ipAddr + ":" + prt, peerInfo);

        condition.notify_all();

        rcvQ_.queue().erase(iter++);
    }
};

int
UdpEndpoint6::receive() {
    TRACE();
    int bytes;
    int peerLen = sizeof(struct sockaddr_in6);
    char buffer[BUFFLEN];
    struct sockaddr_in6 peer;
    ASIO::AsyncIOHandler asioHdl(NW_MAX_EVENTS, "nwPoller6");

    // Make socket non-blocking
    if (Utils::setFdNonBlocking(sockfd_) == -1) {
        LOG(ERROR, "IPv6: Failed to make server socket non-blocking");
    }

    // Create poller object
    if (asioHdl.create() == -1) {
        return -1;
    }

    // Add nw socket to poller object
    if (asioHdl.addFd(sockfd_) == -1) {
        return -1;
    }

    // Create notifier event for stopping thread
    // Main thread will notify if thread has be be
    // cleaned up in case of success or failure
    eventFd_ = eventNotifier_.create(0, EFD_SEMAPHORE);
    if (eventFd_ == -1) {
        LOG(ERROR, "IPv6: Failed to create eventfd");
        return -1;
    }

    // Add event fd to poller object
    if (asioHdl.addFd(eventFd_) == -1) {
        return -1;
    }

    // Main thread loop
    while (true) {
        if (!stopThread_) {
            // Block here waiting for event on poller
            int polledFd = asioHdl.watch();
            if (polledFd == sockfd_) {
                LOG(INFO, "IPv6: UDP socket has become available");
                bytes = recvfrom(sockfd_, buffer, BUFFLEN, 0,
                                 (struct sockaddr *)&peer,
                                 (socklen_t *)&peerLen);

                if (bytes == -1) {
                    LOG(ERROR, "IPv6: recvfrom() failed in receive()");
                    perror("recvfrom");
                    return -1;
                }

                buffer[bytes] = '\0';

                PeerInfo6 peerInfo;
                peerInfo.peer = peer;
                peerInfo.bufferLen = bytes;
                memcpy(peerInfo.buffer, buffer, peerInfo.bufferLen);

                ipAddress ipAddr = sinAddrToStr((void*)&peerInfo.peer.sin6_addr);
                port prt = std::to_string(ntohs(peerInfo.peer.sin6_port));

                LOG(INFO, "IPv6: Received packet from %s:%s", ipAddr.c_str(), prt.c_str());
                LOGT("IPv6: Client sent data : %s:%s", ipAddr.c_str(), prt.c_str());

                rcvQ_.enqueue(ipAddr + ":" + prt, peerInfo);

                // handle connection
                handleConnection();

                LOG(INFO, "Data: %s", buffer);
            } else if (polledFd == eventFd_) {
                if (eventNotifier_.wait(STOP_NW_THREAD)) {
                    LOG(INFO, "IPv6: Nw thread received stop event");
                    stopThread_ = true;
                    // Notify all threads to stop executing
                    condition.notify_all();
                }
            } else {
                LOG(ERROR, "IPv6: Unknown fd polled %d", polledFd);
            }
        } else {
            LOG(INFO, "IPv6: Nw thread stopped" );
            return 0;
        }
    }  // end of while (true)

    return 0;
}

UdpEndpoint6::~UdpEndpoint6() {
    TRACE();

    if (!stopThread_) {
        stopThread_ = true;
        condition.notify_all();
    }

    LOG(INFO, "IPv6: Closing nw socket");

    if (sockfd_ > 0) {
        if (close(sockfd_) == -1) {
            LOG(ERROR, "IPv6: Error closing socket");
            perror("close()");
        }
    }
    sockfd_ = 0;
}

// End of UdpEndpoint6
}  // namespace Network

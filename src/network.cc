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

namespace Network {

// Create different packet queues for sending and receiving packets
// Threads which want to send will enqueue on send queue and threads
// which want to receive will block on receive queue
// Pkt queue for pkts to be sent to client
Queue<PeerData4> globalSendPktQ4;
Queue<PeerData4> globalRcvPktQ4;

// Pkt queue for pkts received from client
Queue<PeerData6> globalSendPktQ6;
Queue<PeerData6> globalRcvPktQ6;

Map<HashKey, IKEv2Session4> globalIKEv2Session4Map;
Map<HashKey, IKEv2Session6> globalIKEv2Session6Map;

// Start of class IKEv2SessionManager4

// IKEv2SessionManager must run in 4 threads
// Pool of threads must handle connection object
//
IKEv2SessionManager4::IKEv2SessionManager4() {
    TRACE();

}

IKEv2SessionManager4::IKEv2SessionManager4(const IKEv2SessionManager4 & other) {
    TRACE();

}

S32
IKEv2SessionManager4::handleSession() {
    TRACE();

    // XXX Do not pass scoped variables to functions expecting reference
    // XXX Scoped variables are deleted at end of scope
    PeerData4::Ptr elem;

    // UdpEndpoint4::receive() will notify when new packet is added
    // Block till rcvQ4 has some packet
    while (true) {

        // If queue has been shutdown return from loop
        if (globalRcvPktQ4.stopped()) {
            return 0;
        }

        // Block until packet is available in receive queue
        // Get element from queue if not empty
        if (!globalRcvPktQ4.getPkt(elem)) {
            continue;
        }

        // Find if hash already exists in connectionMap
        IKEv2Session4::Ptr conn;
        bool found = globalIKEv2Session4Map.find(elem->hash, conn);

        // If it exists enqueue packet in private connection Q
        if (found) {
            LOGT("Session already exists");
            globalSendPktQ4.addPkt(elem);
            ENQUEUE_TIMER_TASK(3000, false, &IKEv2Session4::handleTimeout,
                               conn, elem->hash);
            // check session state
            // call session functions to process packet
            // get processed packet and add to send Q
            // session will hold current state of ike session
            // XXX who will run timer for session?

        // Create new connection and add to connectionMap
        } else {
            LOGT("Creating new session");
            auto conn = IKEv2Session4::Ptr(new IKEv2Session4(elem->hash));
            globalIKEv2Session4Map.add(elem->hash, conn);
            globalSendPktQ4.addPkt(elem);
            ENQUEUE_TIMER_TASK(3000, false, &IKEv2Session4::handleTimeout, c, elem->hash);
        }

        // Process packet here and send reply
        // Packet processing happens in handleSession4
        // context

    }

    return 0;
}

void
IKEv2SessionManager4::shutdown() {
    TRACE();

    globalSendPktQ4.shutdown();
    globalRcvPktQ4.shutdown();

    LOG(INFO, "Stopping session manager 4 thread");
}

// Return global static ikev2 session manager
IKEv2SessionManager4 &
IKEv2SessionManager4::getIKEv2SessionManager4() {
    TRACE();
    static IKEv2SessionManager4 ikev2SessionManager4;
    return ikev2SessionManager4;
}

IKEv2SessionManager4::~IKEv2SessionManager4() {
    TRACE();

    shutdown();

    LOG(INFO, "Sucessfully tore down session manager 4 thread");
}

//End of class IKEv2SessionManager4


// Start of class IKEv2SessionManager6

IKEv2SessionManager6::IKEv2SessionManager6() {
    TRACE();
}

IKEv2SessionManager6::IKEv2SessionManager6(const IKEv2SessionManager6 & other) {
    TRACE();

}

S32
IKEv2SessionManager6::handleSession() {
    TRACE();

    PeerData6::Ptr elem;

    while (true) {

        if (globalRcvPktQ6.stopped()) {
            return 0;
        }

        // Get element from queue if not empty
        if (!globalRcvPktQ6.getPkt(elem)) {
            continue;
        }

        // Find if hash already exists in connectionMap
        IKEv2Session6::Ptr conn;

        bool found = globalIKEv2Session6Map.find(elem->hash, conn);
        // If it exists enqueue packet in private connection Q
        if (found) {
            LOGT("Session already exists");
            globalSendPktQ6.addPkt(elem);
            ENQUEUE_TIMER_TASK(3000, false, &IKEv2Session6::handleTimeout, conn, elem->hash);
            // Create new connection and add to connectionMap
        } else {
            LOGT("Creating new session");
            auto c = IKEv2Session6::Ptr(new IKEv2Session6(elem->hash));
            globalIKEv2Session6Map.add(elem->hash, c);
            globalSendPktQ6.addPkt(elem);
            ENQUEUE_TIMER_TASK(3000, false, &IKEv2Session6::handleTimeout, c, elem->hash);
        }

        // Process packet here and send reply
        // Packet processing happens in handleSession4
        // context
    }

    return 0;
}

void
IKEv2SessionManager6::shutdown() {
    TRACE();

    LOG(INFO, "Stopping session manager 6 thread");

    globalSendPktQ6.shutdown();
    globalRcvPktQ6.shutdown();
}

// Return global static ikev2 session manager
IKEv2SessionManager6 &
IKEv2SessionManager6::getIKEv2SessionManager6() {
    TRACE();
    static IKEv2SessionManager6 ikev2SessionManager6;
    return ikev2SessionManager6;
}

IKEv2SessionManager6::~IKEv2SessionManager6() {
    TRACE();

    shutdown();

    LOG(INFO, "Sucessfully tore down session manager 6 thread");
}

//End of class IKEv2SessionManager6


// Start of class IKEv2Session4
IKEv2Session4::IKEv2Session4(const HashKey & h) {
    TRACE();

    hash_ = h;

    std::vector<std::string> s;
    boost::split(s, hash_, boost::is_any_of("-"));

    ipAddr_ = IpAddress4(s[0]);
    port_ = s[1];

}

S32
IKEv2Session4::handleSession(std::deque<SCHAR *> & pktList) {
    TRACE();
    for (auto & iter : pktList) {
        LOGT("%s", iter);
        std::cout << iter;
    }

    //sendQueue.enqueue(ipAddr_, port_, iter);
    return 0;
}

void
IKEv2Session4::handleTimeout(HashKey hash) {
    TRACE();
    std::unique_lock<std::mutex> lock(sessionMutex_);
    globalIKEv2Session4Map.erase(hash);
    LOGT("%s successfully deleted after timeout", hash.c_str());
}

IKEv2Session4::~IKEv2Session4() {
    TRACE();
}

// End of class IKEv2Session4

// Start of class IKEv2Session6
IKEv2Session6::IKEv2Session6(const HashKey & h) {
    TRACE();

    hash_ = h;
    std::vector<std::string> s;
    boost::split(s, hash_, boost::is_any_of("-"));

    ipAddr_ = IpAddress6(s[0]);
    port_ = s[1];

}

S32
IKEv2Session6::handleSession(std::deque<SCHAR *> & pktList) {
    TRACE();
    for (auto & iter : pktList) {
        LOGT("%s", iter);
        std::cout << iter;
    }

    //sendQueue.enqueue(ipAddr_, port_, iter);
    return 0;
}

void
IKEv2Session6::handleTimeout(HashKey hash) {
    TRACE();
    std::unique_lock<std::mutex> lock(sessionMutex_);
    globalIKEv2Session6Map.erase(hash);
    LOGT("%s successfully deleted after timeout", hash.c_str());
}

IKEv2Session6::~IKEv2Session6() {
    TRACE();
}

// End of class IKEv2Session6

// Start of class UdpEndpoint
UdpEndpoint::UdpEndpoint() : stopThread_(false),
                             eventFd_(-1),
                             sockfd_(-1),
                             eventNotifier_("NetworkNotifier") {
    TRACE();
}

UdpEndpoint::~UdpEndpoint() {
    TRACE();
}

inline void
UdpEndpoint::sourceInterfaceIs(const Interface & intf) {
    TRACE();
    sourceInterface_ = intf;
}

Interface
UdpEndpoint::sourceInterface() const {
    TRACE();
    return sourceInterface_;
}

inline void
UdpEndpoint::ipVersionIs(const IpVersion & version) {
    ipVersion_ = version;
}

inline IpVersion
UdpEndpoint::ipVersion() const {
    return ipVersion_;
}

Synchro::Notifier &
UdpEndpoint::eventNotifier() {
    return eventNotifier_;
}

// End of class UdpEndpoint

// Start of class UdpEndpoint4

UdpEndpoint4::UdpEndpoint4(const IpAddress4 & srvAddr,
                           const NetworkPort & srvPort) {
    TRACE();
    ipVersionIs(IpVersion::IPv4);
    sourceAddress_ = srvAddr;
    sourcePort_ = srvPort;
}

UdpEndpoint4::UdpEndpoint4(const UdpEndpoint4 & other) {
    TRACE();
}


S32
UdpEndpoint4::initUdpEndpoint() {
    TRACE();

    S32 ret = 0;
    S32 reUseAddr = 1;
    SCHAR ipAddr[INET_ADDRSTRLEN];
    struct addrinfo intfHint, *intfInfo, *iter;

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
            LOG(INFO, "IPv4: %s:%d", ipAddr, htons(ipv4->sin_port));
            sourceAddressIs(IpAddress4(ipAddr));
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

S32
UdpEndpoint4::send() {
    TRACE();
    //struct sockaddr_in peer;
    //struct msghdr msg;
    //struct iovec iov;
    //sendmsg
    PeerData4::Ptr elem;

    LOG(INFO, "IPv4: Send thread created successfully");

    // deque packets and send
    while (true) {
        if (stopThread_) {
            return 0;
        }

        // Wait for sendQueue to have packet
        if (globalSendPktQ4.stopped()) {
            LOG(INFO, "UdpEndpoint4::send stopped");
            return 0;
        }

        if (!globalSendPktQ4.getPkt(elem)) {
            continue;
        }

        // sockfd
        // buffer
        // buffer len
        // flags
        // struct sockaddr
        // sockaddr len
        S32 ret = sendto(sockfd_,
                         &elem->buffer,
                         elem->bufferLen,
                         0,
                         (struct sockaddr *)&elem->peer,
                         sizeof(elem->peer));
        if (ret == -1) {
            LOG(ERROR, "IPv4: Error in sendto");
            perror("sendto");
            return -1;
        }

        //globalIKEv2Session4Map.erase(elem->hash);

        LOG(INFO, "IPv4: rcvQ.size() %d", globalRcvPktQ4.queue().size());
        LOG(INFO, "IPv4: sendQ.size() %d", globalSendPktQ4.queue().size());
        LOG(INFO, "IPv4: rcvQ.empty() %d", globalRcvPktQ4.queue().empty());
        LOG(INFO, "IPv4: sendQ.empty() %d", globalSendPktQ4.queue().empty());
    }  // end of while (true)
    return 0;
}


S32
UdpEndpoint4::receive() {
    TRACE();
    S32 bytes;
    S32 peerLen = sizeof(struct sockaddr_in);
    SCHAR buffer[BUFFLEN];
    struct sockaddr_in peer;
    ASIO::AsyncIOHandler asioHdl(NW_MAX_EVENTS, "NetworkPoller4");

    // Make socket non-blocking
    if (Utils::setFdNonBlocking(sockfd_) == -1) {
        LOG(ERROR, "IPv4: Failed to make server socket non-blocking");
        return -1;
    }

    // Create poller object
    if (asioHdl.createPoller() == -1) {
        return -1;
    }

    // Add nw socket to poller object
    if (asioHdl.addFd(sockfd_) == -1) {
        return -1;
    }

    // Create notifier event for stopping thread
    // Main thread will notify if thread has be be
    // cleaned up in case of success or failure
    eventFd_ = eventNotifier_.createNotifier(0, EFD_SEMAPHORE);
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
            S32 polledFd = asioHdl.watchFds();
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

                auto peerData = PeerData4::Ptr(new PeerData4());

                peerData->peer = peer;
                peerData->bufferLen = bytes;
                memcpy(peerData->buffer, buffer, peerData->bufferLen);

                IpAddress4 ipAddr = sinAddrToStr((void*)&peerData->peer.sin_addr);
                NetworkPort port = std::to_string(ntohs(peerData->peer.sin_port));

                peerData->hash = ipAddr.toString() + "-" + port;

                LOG(INFO, "IPv4: Received packet from %s:%s", ipAddr.toRawString(), port.c_str());
                LOGT("IPv4: Client sent data : %s:%s", ipAddr.toRawString(), port.c_str());

                // Add pkt to receive queue and notify handler
                globalRcvPktQ4.addPkt(peerData);

            } else if (polledFd == eventFd_) {
                if (eventNotifier_.readEvent(STOP_NW_THREAD)) {
                    LOG(INFO, "IPv4: Nw thread received stop event");
                    stopThread_ = true;
                    return 0;
                    // Notify all threads to stop executing
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

inline void
UdpEndpoint4::sourceAddressIs(const IpAddress4 & addr) {
    TRACE();
    sourceAddress_ = addr;
}

inline void
UdpEndpoint4::peerAddressIs(const IpAddress4 & addr) {
    TRACE();
    peerAddress_ = addr;
}

bool
UdpEndpoint4::isIpAddress(const IpAddress4 & ipAddr) {
    TRACE();
    S32 result = 0;

    struct sockaddr_in sa;
    result = inet_pton(AF_INET, ipAddr.toRawString(), &(sa.sin_addr));

    return result != 0;
}

IpAddress4
UdpEndpoint4::peerAddress() {
    TRACE();
    return peerAddress_;
}


IpAddress4
UdpEndpoint4::sourceAddress() {
    TRACE();
    return sourceAddress_;
}


IpAddress4
UdpEndpoint4::sinAddrToStr(void * peer) {
    TRACE();

    SCHAR straddr[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, peer, straddr, sizeof(straddr)) == nullptr) {
        perror("inet_ntop");
        return nullptr;
    }

    return IpAddress4(straddr);

}

UdpEndpoint4::~UdpEndpoint4() {
    TRACE();

    if (!stopThread_) {
        stopThread_ = true;
    }

    LOG(INFO, "IPv4: Closing nw socket");

    if (sockfd_ > 0) {
        if (close(sockfd_) == -1) {
            LOG(ERROR, "IPv4: Error closing socket");
            perror("close()");
        }
    }
    sockfd_ = -1;
}

// End of class UdpEndpoint4

// Start of UdpEndpoint6
UdpEndpoint6::UdpEndpoint6(const IpAddress6 & srvAddr,
                           const NetworkPort & srvPort) {
    TRACE();
    ipVersionIs(IpVersion::IPv6);
    sourceAddress_ = srvAddr;
    sourcePort_ = srvPort;
                           }

// XXX Remove this
UdpEndpoint6::UdpEndpoint6(const UdpEndpoint6 & other) {
    TRACE();
}

S32
UdpEndpoint6::initUdpEndpoint() {
    TRACE();

    S32 ret = 0;
    S32 reUseAddr = 1;
    SCHAR ipAddr[INET6_ADDRSTRLEN];
    struct addrinfo intfHint;
    struct addrinfo *intfInfo;
    struct addrinfo *iter;

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

            if (bind(sockfd_, (struct sockaddr *)ipv6, sizeof(*ipv6)) == -1) {
                perror("IPv6: bind failed");
                close(sockfd_);
                continue;
            }

            LOG(INFO, "IPv6: Successfully bound to port %s", IKEV2_UDP_PORT);
            LOG(INFO, "IPv6: Socket is ready to receive / send data");

            // Convert ip address to string
            inet_ntop(iter->ai_family, &(ipv6->sin6_addr), ipAddr, sizeof(ipAddr));
            LOG(INFO, "IPv6: %s:%d", ipAddr, htons(ipv6->sin6_port));
            sourceAddressIs(IpAddress6(ipAddr));
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

S32
UdpEndpoint6::send() {
    TRACE();
    //struct sockaddr_in peer;
    //struct msghdr msg;
    //struct iovec iov;
    //sendmsg
    PeerData6::Ptr elem;

    LOG(INFO, "IPv6: Send thread created successfully");

    // deque packets and send
    while (true) {
        if (stopThread_) {
            return 0;
        }

        // Wait for sendQueue to have packet
        if (globalSendPktQ6.stopped()) {
            LOG(INFO, "UdpEndpoint6::send stopped");
            return 0;
        }

        // Returns reference to first pkt in queue
        if (!globalSendPktQ6.getPkt(elem)) {
            continue;
        }

        S32 ret = sendto(sockfd_,
                         &elem->buffer,
                         elem->bufferLen,
                         0,
                         (struct sockaddr *)&elem->peer,
                         sizeof(elem->peer));
        if (ret == -1) {
            LOG(ERROR, "IPv6: Error in sendto");
            perror("sendto");
            return -1;
        }

        //globalIKEv2Session6Map.erase(elem->hash);

        LOG(INFO, "IPv6: rcvQueue_.queue().size() %d", globalRcvPktQ6.queue().size());
        LOG(INFO, "IPv6: sendQ_.queue().size() %d", globalSendPktQ6.queue().size());
        LOG(INFO, "IPv6: rcvQueue_.queue().empty() %d", globalRcvPktQ6.queue().empty());
        LOG(INFO, "IPv6: sendQ_.queue().empty() %d", globalSendPktQ6.queue().empty());

    }  // end of while (true)
    return 0;
}

S32
UdpEndpoint6::receive() {
    TRACE();
    S32 bytes;
    S32 peerLen = sizeof(struct sockaddr_in6);
    SCHAR buffer[BUFFLEN];
    struct sockaddr_in6 peer;
    ASIO::AsyncIOHandler asioHdl(NW_MAX_EVENTS, "NetworkPoller6");

    // Make socket non-blocking
    if (Utils::setFdNonBlocking(sockfd_) == -1) {
        LOG(ERROR, "IPv6: Failed to make server socket non-blocking");
    }

    // Create poller object
    if (asioHdl.createPoller() == -1) {
        return -1;
    }

    // Add nw socket to poller object
    if (asioHdl.addFd(sockfd_) == -1) {
        return -1;
    }

    // Create notifier event for stopping thread
    // Main thread will notify if thread has be be
    // cleaned up in case of success or failure
    eventFd_ = eventNotifier_.createNotifier(0, EFD_SEMAPHORE);
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
            S32 polledFd = asioHdl.watchFds();
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

                auto peerData = PeerData6::Ptr(new PeerData6());

                peerData->peer = peer;
                peerData->bufferLen = bytes;
                memcpy(peerData->buffer, buffer, peerData->bufferLen);

                IpAddress6 ipAddr = sinAddrToStr((void*)&peerData->peer.sin6_addr);
                NetworkPort port = std::to_string(ntohs(peerData->peer.sin6_port));
                peerData->hash = ipAddr.toString() + "-" + port;

                LOG(INFO, "IPv6: Received packet from %s:%s", ipAddr.toRawString(), port.c_str());

                globalRcvPktQ6.addPkt(peerData);

                LOG(INFO, "Data: %s", buffer);
            } else if (polledFd == eventFd_) {
                if (eventNotifier_.readEvent(STOP_NW_THREAD)) {
                    LOG(INFO, "IPv6: Nw thread received stop event");
                    stopThread_ = true;
                    return 0;
                    // Notify all threads to stop executing
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

inline void
UdpEndpoint6::sourceAddressIs(const IpAddress6 & addr) {
    TRACE();
    sourceAddress_ = addr;
}

inline void
UdpEndpoint6::peerAddressIs(const IpAddress6 & addr) {
    TRACE();
    peerAddress_ = addr;
}

IpAddress6
UdpEndpoint6::peerAddress() {
    TRACE();
    return peerAddress_;
}

IpAddress6
UdpEndpoint6::sourceAddress() {
    TRACE();
    return sourceAddress_;
}

bool
UdpEndpoint6::isIpAddress(const IpAddress6 & ipAddr) {
    TRACE();
    S32 result = 0;

    struct sockaddr_in6 sa;
    result = inet_pton(AF_INET6, ipAddr.toRawString(), &(sa.sin6_addr));

    return result != 0;
}

IpAddress6
UdpEndpoint6::sinAddrToStr(void * peer) {

    SCHAR straddr[INET6_ADDRSTRLEN];
    if (inet_ntop(AF_INET6, peer, straddr, sizeof(straddr)) == nullptr) {
        perror("inet_ntop");
        return nullptr;
    }

    return IpAddress6(straddr);
}

UdpEndpoint6::~UdpEndpoint6() {
    TRACE();

    if (!stopThread_) {
        stopThread_ = true;
    }

    LOG(INFO, "IPv6: Closing nw socket");

    if (sockfd_ > 0) {
        if (close(sockfd_) == -1) {
            LOG(ERROR, "IPv6: Error closing socket");
            perror("close()");
        }
    }
    sockfd_ = -1;
}

// End of UdpEndpoint6
}  // namespace Network

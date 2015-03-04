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

#include <netdb.h>   // getaddrinfo()
#include <unistd.h>  // close()
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <mutex>
#include <memory>
#include <condition_variable>
#include <boost/algorithm/string.hpp>

#include "Logging.hh"
#include "BasicTypes.hh"
#include "Synchro.hh"
#include "Asyncio.hh"
#include "Utils.hh"
#include "Timer.hh"
#include "Queue.hh"
#include "Map.hh"
#include "BasicTypes.hh"
#include "IpAddress.hh"

#define BUFFLEN 1024

#define SERVER_ADDR4 "127.0.0.1"
#define SERVER_ADDR6 "::"

#define IKEV2_UDP_PORT "34455"

using NetworkPort = std::string;
using Interface = std::string;
using HashKey = std::string;

enum class IpVersion { IPv4, IPv6 };

namespace Network {

const S32 STOP_NW_THREAD = 1;

const S32 NW_MAX_EVENTS = 3;

// Main thread will read the single socket for data / packet
// and enqueue packet in packet queue
// Global packet queue

struct PeerData4 {
    HashKey hash;
    S32 bufferLen;
    SCHAR buffer[BUFFLEN];
    struct sockaddr_in peer;
    using Ptr = std::shared_ptr<PeerData4>;
};

struct PeerData6 {
    HashKey hash;
    S32 bufferLen;
    SCHAR buffer[BUFFLEN];
    struct sockaddr_in6 peer;
    using Ptr = std::shared_ptr<PeerData6>;
};

class ProtocolSession {
 public:
     ProtocolSession();
     ~ProtocolSession();
 private:
};

// Session abstraction to handle per client connection
// Single connection can have multiple ike sa's and ipsec sa's
//
class IKEv2Session4 {
 public:
    using Ptr = std::shared_ptr<IKEv2Session4>;
    IKEv2Session4(const HashKey & h);
    ~IKEv2Session4();
    S32 handleSession(std::deque<SCHAR *> & pktList);
    void handleTimeout(HashKey);
 private:
    HashKey hash_;
    IpAddress4 ipAddr_;
    NetworkPort port_;
    std::mutex sessionMutex_;
    // ike sa
    // ipsec sa
    // ip addr / port
    // negotiated algs
    // keys
    // crypto info
    // etc
};

class IKEv2Session6 {
 public:
    using Ptr = std::shared_ptr<IKEv2Session6>;
    IKEv2Session6(const HashKey & h);
    ~IKEv2Session6();
    S32 handleSession(std::deque<SCHAR *> & pktList);
    void handleTimeout(HashKey);
 private:
    HashKey hash_;
    IpAddress6 ipAddr_;
    NetworkPort port_;
    std::mutex sessionMutex_;
    // ike sa
    // ipsec sa
    // ip addr / port
    // negotiated algs
    // keys
    // crypto info
    // etc
};

// Forward declaration
class UdpEndpoint4;
class UdpEndpoint6;

// Per client job dispatcher which will take packet from Queue
// and enqueue the packet in one of the threads
// XXX What if all threads are occupied?
// IKEv2SessionManager will create connection for new client else
// will send packet to an existing connection
class IKEv2SessionManager4 final {
 public:
    IKEv2SessionManager4();
    ~IKEv2SessionManager4();

    S32 handleSession();
    void shutdown();
    static IKEv2SessionManager4 & getIKEv2SessionManager4();

    IKEv2SessionManager4(const IKEv2SessionManager4 &);
    IKEv2SessionManager4(IKEv2SessionManager4 &&);
    IKEv2SessionManager4 & operator=(const IKEv2SessionManager4 &);
    IKEv2SessionManager4 & operator=(IKEv2SessionManager4 &&);
 private:
    std::mutex queueMutex_;
};

class IKEv2SessionManager6 final {
 public:
    IKEv2SessionManager6();
    ~IKEv2SessionManager6();

    S32 handleSession();
    void shutdown();
    static IKEv2SessionManager6 & getIKEv2SessionManager6();

    IKEv2SessionManager6(const IKEv2SessionManager6 &);
    IKEv2SessionManager6(IKEv2SessionManager6 &&);
    IKEv2SessionManager6 & operator=(const IKEv2SessionManager6 &);
    IKEv2SessionManager6 & operator=(IKEv2SessionManager6 &&);
 private:
    std::mutex queueMutex_;
};

// Abstract base class for network endpoints
class UdpEndpoint {
 public:
    UdpEndpoint();
    virtual ~UdpEndpoint();

    Synchro::Notifier & eventNotifier();
    void ipVersionIs(const IpVersion & version);
    IpVersion ipVersion() const;
    void sourceInterfaceIs(const Interface & intf);
    Interface sourceInterface() const;
 protected:
    bool stopThread_;
    S32 eventFd_;
    S32 sockfd_;
    Interface sourceInterface_;
    IpVersion ipVersion_;
    Synchro::Notifier eventNotifier_;
    std::mutex queueMutex_;
};

class UdpEndpoint4 : public UdpEndpoint {
 public:
    UdpEndpoint4(const IpAddress4 & destination,
                 const NetworkPort & destPort);
    ~UdpEndpoint4();

    // Copy constructor
    UdpEndpoint4(const UdpEndpoint4 & other);
    // Copy assignment operator
    UdpEndpoint4 & operator=(const UdpEndpoint4 & other);

    S32 initUdpEndpoint();
    S32 send();
    S32 receive();

    void sourceAddressIs(const IpAddress4 & addr);
    void peerAddressIs(const IpAddress4 & addr);
    bool isIpAddress(const IpAddress4 & addr);

    IpAddress4 peerAddress();
    IpAddress4 sourceAddress();
    IpAddress4 sinAddrToStr(void * peer);
 private:
    NetworkPort peerPort_;
    NetworkPort sourcePort_;
    IpAddress4 sourceAddress_;
    IpAddress4 peerAddress_;
};

class UdpEndpoint6 : public UdpEndpoint {
 public:
    UdpEndpoint6(const IpAddress6 & destination,
                 const NetworkPort & destPort);
    ~UdpEndpoint6();

    // Copy constructor
    UdpEndpoint6(const UdpEndpoint6 & other);
    // Copy assignment operator
    UdpEndpoint6 & operator=(const UdpEndpoint6 & other);

    S32 initUdpEndpoint();
    S32 send();
    S32 receive();

    void sourceAddressIs(const IpAddress6 & addr);
    void peerAddressIs(const IpAddress6 & addr);
    bool isIpAddress(const IpAddress6 & addr);

    IpAddress6 peerAddress();
    IpAddress6 sourceAddress();
    IpAddress6 sinAddrToStr(void * peer);
 private:
    NetworkPort peerPort_;
    NetworkPort sourcePort_;
    IpAddress6 peerAddress_;
    IpAddress6 sourceAddress_;

};

}  // namespace Network

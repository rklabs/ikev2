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

#include "logging.hh"
#include "basictypes.hh"
#include "synchro.hh"
#include "asyncio.hh"
#include "processor.hh"
#include "utils.hh"
#include "timer.hh"
#include "queue.hh"
#include "map.hh"

#define BUFFLEN 1024

#define SERVER_ADDR4 "127.0.0.1"
#define SERVER_ADDR6 "::"

#define IKEV2_UDP_PORT "34455"

typedef std::string ipAddress;
typedef std::string port;
typedef std::string interface;
typedef std::string ipVersion;
typedef std::string hashKey;

enum class AddressFamily { IPv4, IPv6 };

namespace Network {

const int STOP_NW_THREAD = 1;

const int NW_MAX_EVENTS = 3;

// Main thread will read the single socket for data / packet
// and enqueue packet in packet queue
// Global packet queue

struct PeerData4 {
    hashKey hash;
    int bufferLen;
    char buffer[BUFFLEN];
    struct sockaddr_in peer;
};

struct PeerData6 {
    hashKey hash;
    int bufferLen;
    char buffer[BUFFLEN];
    struct sockaddr_in6 peer;
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
    IKEv2Session4(const hashKey & h);
    ~IKEv2Session4();
    int handleSession(std::deque<char *> & pktList);
    void handleTimeout(hashKey);
 private:
    hashKey hash_;
    ipAddress ipAddr_;
    port port_;
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
    IKEv2Session6(const hashKey & h);
    ~IKEv2Session6();
    int handleSession(std::deque<char *> & pktList);
    void handleTimeout(hashKey);
 private:
    hashKey hash_;
    ipAddress ipAddr_;
    port port_;
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

    int handleSession();
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

    int handleSession();
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
class NetworkEndpoint {
 public:
    NetworkEndpoint();
    ~NetworkEndpoint();
 private:
};

class UdpEndpoint : public NetworkEndpoint {
 public:
    UdpEndpoint(const ipAddress &destination,
                const port &destPort,
                AddressFamily addrFamily);
    ~UdpEndpoint();
    void sourceAddressIs(const ipAddress & addr);
    void peerAddressIs(const ipAddress & addr);
    void sourceInterfaceIs(const interface & intf);
    void ipVersionIs(const ipVersion & version);
    bool isIpAddress(const ipAddress & addr);
    ipAddress peerAddress();
    interface sourceInterface();
    ipAddress sourceAddress();
    Synchro::Notifier & eventNotifier();
    ipAddress sinAddrToStr(void * peer);

 protected:
    bool stopThread_;
    int eventFd_;
    int sockfd_;
    ipAddress peerAddress_;
    port peerPort_;
    interface sourceInterface_;
    ipAddress sourceAddress_;
    ipVersion ipVersion_;
    AddressFamily addrFamily_;
    Synchro::Notifier eventNotifier_;
    std::mutex queueMutex_;
};

class UdpEndpoint4 : public UdpEndpoint {
 public:
    UdpEndpoint4(const ipAddress & destination,
                 const port & destPort);
    ~UdpEndpoint4();

    // Copy constructor
    UdpEndpoint4(const UdpEndpoint4 & other);
    // Copy assignment operator
    UdpEndpoint4 & operator=(const UdpEndpoint4 & other);

    int initUdpEndpoint();
    int send();
    int receive();
 private:

};

class UdpEndpoint6 : public UdpEndpoint {
 public:
    UdpEndpoint6(const ipAddress &destination,
                 const port &destPort);
    ~UdpEndpoint6();

    // Copy constructor
    UdpEndpoint6(const UdpEndpoint6 & other);
    // Copy assignment operator
    UdpEndpoint6 & operator=(const UdpEndpoint6 & other);

    int initUdpEndpoint();
    int send();
    int receive();

 private:

};

}  // namespace Network

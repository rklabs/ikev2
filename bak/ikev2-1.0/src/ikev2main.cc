/*
 * ikev2_main.cc
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

#include <config.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>

#include "network.hh"
#include "logging.hh"
#include "ikev2config.hh"
#include "threadpool.hh"
#include "crypto.hh"
#include "exception.hh"
#include "utils.hh"

const std::size_t MAX_NW_THREADS = 3;

Crypto::CryptoPluginInterface *cryptoPlugin = new Crypto::OpensslPlugin();
IKEv2::Config cfg;

// Create vector of v4 / v6 threads to handle connections on same fd
std::vector<Network::UdpEndpoint4> udpEndpoints4;
std::vector<Network::UdpEndpoint6> udpEndpoints6;

void cleanup(int status) {
    // Cleanup config thread
    cfg.eventNotifier().notify(STOP_CFG_THREAD);

    // Cleanup nw threads
    for (auto & iter : udpEndpoints4) {
        iter.eventNotifier().notify(STOP_NW_THREAD);
    }

    for (auto & iter : udpEndpoints6) {
        iter.eventNotifier().notify(STOP_NW_THREAD);
    }

    // Cleanup crypto plugin
    if (cryptoPlugin) {
        LOG(INFO, "Cleaning up cryptoPlugin");
        delete cryptoPlugin;
    }

#if IKEV2_GCOV == 1
    // Make sure coverage data is flushed
    Utils::__gcov_flush();
#endif

    exit(status);
}

static void sigIntHandler(int signal) {
    LOG(INFO, "Caught SIGINT!");
    LOGT("Shutting down IKEv2. Please wait!");

    cleanup(EXIT_SUCCESS);
}

static void sigTermHandler(int signal) {
    LOG(INFO, "Received signal %d: %s. Shutting down", signal, strsignal(signal));
}

static int setupSigIntHandler() {
    sigset_t sigset;
    struct sigaction sigInfoSigInt;
    struct sigaction sigInfoSigTerm;

    sigemptyset(&sigset);

    sigInfoSigInt.sa_handler = sigIntHandler;
    sigInfoSigInt.sa_mask = sigset;
    sigInfoSigInt.sa_flags = SA_RESTART;

    sigInfoSigTerm.sa_handler = sigTermHandler;
    sigInfoSigTerm.sa_mask = sigset;
    sigInfoSigTerm.sa_flags = SA_RESTART;

    sigaction(SIGINT, &sigInfoSigTerm, NULL);
    sigaction(SIGTERM, &sigInfoSigTerm, NULL);
    sigaction(SIGINT, &sigInfoSigInt, NULL);

    return 0;
}

static void exitHandler() {
    TRACE();
}

int main(int argc, char *argv[]) {
    TRACE();
    LOGT("Starting IKEv2!");
    LOGT("IKEv2 daemon is running with pid %d", getpid());

    // Make all I/O unbuffered
    setvbuf(stdin, nullptr, _IONBF, 0);

    /// Register exit handler
    atexit(&exitHandler);

    // Handle SIGINT
    setupSigIntHandler();

    // Make sure core is dumped if any program error occurs
    Utils::setResourceLimit();

    // Create v4 / v6 endpoints to receive packets
    for (std::size_t _ = 0 ; _ < MAX_NW_THREADS ; _++) {
        udpEndpoints4.push_back(Network::UdpEndpoint4(SERVER_ADDR4, IKEV2_UDP_PORT));
        udpEndpoints6.push_back(Network::UdpEndpoint6(SERVER_ADDR6, IKEV2_UDP_PORT));
    }

    // Create and bind socket to start sending / receiving
    for (auto & iter : udpEndpoints4) {
        iter.initUdpEndpoint();
    }

    for (auto & iter : udpEndpoints6) {
        iter.initUdpEndpoint();
    }

    // Create multiple UdpEndpoint to handle same fd
    // Unique epoll instance in each thread
    // same queue for each thread or multiple queues ?
    // Create epoll class to factor common code
    std::vector<std::future<int>> results;
    // Run config task to read and handle config file changes
    results.push_back(ENQUEUE_TASK(&IKEv2::Config::confFileWatcher, &cfg));

    // Now for each udp endpoint created start receive and send thread
    for (auto & iter : udpEndpoints4) {
        results.push_back(ENQUEUE_TASK(&Network::UdpEndpoint4::receive, &iter));
        results.push_back(ENQUEUE_TASK(&Network::UdpEndpoint4::send, &iter));
    }

    for (auto & iter : udpEndpoints6) {
        results.push_back(ENQUEUE_TASK(&Network::UdpEndpoint6::receive, &iter));
        results.push_back(ENQUEUE_TASK(&Network::UdpEndpoint6::send, &iter));
    }

    // There will be only one receive thread / main thread for port 500
    // XXX Can multiple threads work for same fd?

    // Wait for all threads to complete
    for (auto & iter : results) {
        LOG(INFO, "%d", iter.get());
    }

    cleanup(EXIT_SUCCESS);

    return 0;
}


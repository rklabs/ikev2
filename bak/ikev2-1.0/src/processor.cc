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

#include "processor.hh"

namespace Processor {

Connection::Connection(const std::string & h) : hash_(h) {
    TRACE();

    std::vector<std::string> s;
    boost::split(s, hash_, boost::is_any_of(":"));

    ipAddr_ = s[0];
    port_ = s[1];

}

int Connection::handleConnection(std::deque<char*> & pktList) {
    TRACE();
    for (auto & iter : pktList) {
        LOGT("%s", iter);
        iter = iter;
    }

    //sendPacketQueue.enqueue(ipAddr_, port_, iter);
    return 0;
}

Connection::~Connection() {
    TRACE();
}


JobDispatcher::JobDispatcher() {
    TRACE();
    stopThread_ = false;
}

int JobDispatcher::processPackets() {
    TRACE();
    /*while (!stopThread_) {
        // Need to use conditional variable to block until packet is queued
        // Iterate over packets in workMap
        for (auto & iter : rcvPacketQueue.workMap) {
            LOGT("first %s", iter.first.c_str())

            // If connection already exists then add packet
            // to existing connection
            auto conn = connectionList.find(iter.first);
            if (conn != connectionList.end()) {
                conn->second.handleConnection(iter.second);
            // Create new connection and add to connectionList
            } else {
                Connection c(iter.first);
                connectionList.insert({{iter.first, c}});
                c.handleConnection(iter.second);
            }

            if(!iter.second.empty()) {
                // Iterate over all packets
                // List of packet recevied for single client
                for (auto pIter : iter.second) {
                    LOGT("%s", pIter);
                }
            }
        }
    }*/
    return 0;
}

void JobDispatcher::shutdown() {
    TRACE();
    stopThread_ = true;
}

JobDispatcher::~JobDispatcher() {
    TRACE();
}



// create list of Connections
// should connections be blocked waiting for work
}

/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
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
#include "src/network.hh"
#include "src/logging.hh"
#include "src/config.hh"
#include "src/job.hh"
#include "src/crypto.hh"
#include "src/exception.hh"
#include "src/utils.hh"

int main() {
    ikev2::network::udpSocket skt;
    ikev2::config cfg;
    ikev2::job workQueue;
    ikev2::crypto::cryptoPluginInterface *cryptoPlugin = \
                                            new ikev2::crypto::opensslPlugin();

    // Make sure core is dumped if any program error occurs
    setResourceLimit();

    delete cryptoPlugin;

    return 0;
}


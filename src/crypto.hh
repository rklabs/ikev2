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

#ifndef _SRC_IKEV2_CRYPTO_H_
#define _SRC_IKEV2_CRYPTO_H_

#include <openssl/evp.h>

#include "src/logging.hh"

namespace ikev2 {
namespace crypto {
    class cipher {
     public:
        enum CipherType {
            DES,
            DES3,
            AES
        };
        cipher();
        ~cipher();
        int encrypt();
        int decrypt();
     private:
    };

    class hash {
     public:
        enum HashType {
            MD5,
            SHA1
        };
        hash();
        ~hash();
        int hmac();
    };

    class dh {
     public:
        enum DHGROUPS {
            DH_GROUP1,
            DH_GROUP3,
            DH_GROUP5
        };
        dh();
        ~dh();
        int generateKey();
        int computeKey();
        int generateParams();
    };

    class cryptoPluginInterface {
     protected:
        cipher cipherObj;
        hash hashObj;
        dh dhObj;
     public:
         virtual void init()=0;
         cryptoPluginInterface();
         virtual ~cryptoPluginInterface();

    };

    class opensslPlugin : public cryptoPluginInterface {
     public:
        void init();
        opensslPlugin();
        ~opensslPlugin();
    };

    class cryptoppPlugin : public cryptoPluginInterface {
     public:
         void init();
         cryptoppPlugin();
         ~cryptoppPlugin();

    };
}  // namespace crypto
}  // namespace ikev2

#endif  // _SRC_IKEV2_CRYPTO_H_

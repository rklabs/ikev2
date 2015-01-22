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

#include <openssl/evp.h>
#include <vector>
#include <map>

#include "logging.hh"

namespace Crypto {
class Cipher {
    public:
    enum CipherType {
        DES,
        DES3,
        AES
    };
    Cipher();
    ~Cipher();
    int encrypt();
    int decrypt();
    private:
};

class Hash {
    public:
    enum HashType {
        MD5,
        SHA1
    };
    Hash();
    ~Hash();
};

class Hmac {
    public:
    Hmac();
    ~Hmac();
};

class DH {
    public:
    enum DHGROUPS {
        DH_GROUP1,
        DH_GROUP3,
        DH_GROUP5
    };
    DH();
    ~DH();
    int generateKey();
    int computeKey();
    int generateParams();
};

// This class is abstract base class for cryto library.
// It provides common objects(encryption, hash, dh, pki)
// and methods present in any crypto library
class CryptoPluginInterface {
    protected:
    std::map<int, Cipher> ciphers;
    std::map<int, Hash> hashAlgs;
    std::map<int, Hmac> hmacAlgs;
    std::map<int, DH> diffieHellmanGrps;
    public:
        virtual void init()=0;
        CryptoPluginInterface();
        virtual ~CryptoPluginInterface();
};

class OpensslPlugin : public CryptoPluginInterface {
    public:
    void init();
    OpensslPlugin();
    ~OpensslPlugin();
};

class CryptoppPlugin : public CryptoPluginInterface {
    public:
        void init();
        CryptoppPlugin();
        ~CryptoppPlugin();
};
}  // namespace crypto

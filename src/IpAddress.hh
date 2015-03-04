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

// This file implements abstraction for ip address
#pragma once

#include <iostream>
#include <boost/asio/ip/address.hpp>

class IpAddress4 {
 public:
    using IpAddressv4_ = boost::asio::ip::address_v4;

    IpAddress4();
    IpAddress4(const char * str);
    IpAddress4(const std::string & str);
    bool isMulticast() const;
    std::string toString() const;
    const char * toRawString() const;
 private:
    IpAddressv4_ ipAddr_;
};

inline IpAddress4::IpAddress4() {

}

inline IpAddress4::IpAddress4(const char *str) {
    ipAddr_ = IpAddressv4_::from_string(str);
}

inline IpAddress4::IpAddress4(const std::string & str) {
    ipAddr_ = IpAddressv4_::from_string(str);
}

inline bool IpAddress4::isMulticast() const {
    return ipAddr_.is_multicast();
}

inline std::string IpAddress4::toString() const {
    return ipAddr_.to_string();
}

inline const char * IpAddress4::toRawString() const {
    return ipAddr_.to_string().c_str();
}

template <typename Elem, typename Traits>
std::basic_ostream<Elem, Traits> & operator<<(
    std::basic_ostream<Elem, Traits> & os, const IpAddress4 & addr) {
    os << addr.toString();
    return os;
}

class IpAddress6 {
 public:
    using IpAddressv6_ = boost::asio::ip::address_v6;

    IpAddress6();
    IpAddress6(const char * str);
    IpAddress6(const std::string & str);
    bool isMulticast() const;
    std::string toString() const;
    const char * toRawString() const;
 private:
    IpAddressv6_ ipAddr_;
};

inline IpAddress6::IpAddress6() {

}

inline IpAddress6::IpAddress6(const char *str) {
    ipAddr_ = IpAddressv6_::from_string(str);
}

inline IpAddress6::IpAddress6(const std::string & str) {
    ipAddr_ = IpAddressv6_::from_string(str);
}

inline bool IpAddress6::isMulticast() const {
    return ipAddr_.is_multicast();
}

inline std::string IpAddress6::toString() const {
    return ipAddr_.to_string();
}

inline const char * IpAddress6::toRawString() const {
    return ipAddr_.to_string().c_str();
}

template <typename Elem, typename Traits>
std::basic_ostream<Elem, Traits> & operator<<(
    std::basic_ostream<Elem, Traits> & os, const IpAddress6 & addr) {
    os << addr.toString();
    return os;
}

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
#ifndef _SRC_IKEV2_SA_H_
#define _SRC_IKEV2_SA_H_

#include "src/logging.hh"

namespace ikev2 {
namespace sa {

        class ikev2sa {
            ikev2sa();
            ~ikev2sa();
        };

        class ipsecsa {
            ipsecsa();
            ~ipsecsa();
        };

}  // namespace sa
}  // namespace ikev2


#endif  // _SRC_IKEV2_SA_H_

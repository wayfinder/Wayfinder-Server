/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef CLIENTRIGHTS_H
#define CLIENTRIGHTS_H

#include "config.h"
#include <set>

class UserUser;
class UserRight;

/// contains client rights and its utility functions
namespace ClientRights {

/// describes what client can do
enum Rights {
   ROUTE,  ///< can route
   CONNECT_GPS, ///< can connect gps
   POSITIONING, ///< can use locator
   FLEET, ///< can use fleet...
   TRAFFIC, ///< can see traffic
   NBR_RIGHTS, ///< keep last
};

typedef set<Rights> ClientRights;

/**
 * Determines if user rights can be used with the client right
 * @param clientRight the client right
 * @param userRight users right
 * @return true if clientRight can be used with the current userRight
 */
bool canUseRight( Rights clientRight, const UserRight& userRight );

/**
 * "converts" user rights to client rights
 * @param user the user from which we should get rights from
 * @return a uniq set of client rights
 */
ClientRights getClientRights( const UserUser& user );

}

#endif // CLIENTRIGHTS_H

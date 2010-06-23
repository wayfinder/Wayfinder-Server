/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SERVERPORT_H
#define SERVERPORT_H

#include "config.h"

#include "ServerTypes.h"

namespace ServerPort {

// The "default-ports"
// Remember to change in the Supervisor (SupervisorStorage.cpp)
// if any changes are done here!

static const uint16 NAVIGATOR_FIRST_PORT = 15500;
static const uint16 TRISS_FIRST_PORT = 15600;
static const uint16 DEFAULT_FIRST_PORT = 15700;


/**
 * Returns the default port for a specific Server type.
 *
 * @param The type of server.
 * @return The port used by this server.
 */
inline uint16 getPort( ServerTypes::servertype_t type ) {
   using namespace ServerTypes;

   switch ( type ) {
   case NAVIGATOR:
      return NAVIGATOR_FIRST_PORT;
   case TRISS :
      return TRISS_FIRST_PORT;
   default:
      return DEFAULT_FIRST_PORT;
   }
}

} // ServerPort

#endif // SERVERPORT_H

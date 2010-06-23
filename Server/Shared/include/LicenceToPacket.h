/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef LICENCETOPACKET_H
#define LICENCETOPACKET_H

#include "config.h"
#include "UserPacket.h"
#include <memory>

// Forwards
class UserLicenceKey;

/**
 * Packet for sending a user and licence/hardware key to UM to be
 * connected.
 *
 */
class LicenceToRequestPacket : public UserRequestPacket {
public:
   /**
    * This request packet handles the request for getting stored user data
    */
   LicenceToRequestPacket( uint32 UIN, const UserLicenceKey* licence );

   /**
    * Get the licence key.
    */
   auto_ptr<UserLicenceKey> getLicenceKey() const;
};

/**
 * Reply with status of how the moving of licence/hardware key to user went.
 *
 */
class LicenceToReplyPacket : public UserReplyPacket {
public:
   /**
    * Reply with status.
    * Possible statuses are OK, NOTOK some error occured, NOT_ALLOWED if
    * may not remove key from owner.
    *
    * @param req The request with packetID and requestID.
    * @param ownerUIN The UIN of the owner of the key, possibly same as in
    *        request, 0 if no owner.
    */
   LicenceToReplyPacket( const LicenceToRequestPacket* req, uint32 ownerUIN );

   /**
    * Get the ownerUIN. 
    */
   uint32 getOwnerUIN() const;
};

#endif // LICENCETOPACKET_H

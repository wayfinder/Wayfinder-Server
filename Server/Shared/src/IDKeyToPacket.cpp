/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "IDKeyToPacket.h"
#include "UserIDKey.h"


IDKeyToRequestPacket::IDKeyToRequestPacket( uint32 UIN, 
                                            const UserIDKey* key,
                                            const UserIDKey* removeIDkey )
   : UserRequestPacket( USER_REQUEST_HEADER_SIZE + key->getSize() + 4,
                        PACKETTYPE_ID_KEY_TO_REQUEST,
                        UIN )
{
   int pos = USER_REQUEST_HEADER_SIZE;
   if ( removeIDkey != NULL ) {
      incWriteLong( pos, removeIDkey->getID() );
   } else {
      incWriteLong( pos, 0 );
   }
   const_cast<UserIDKey*>( key )->packInto( this, pos );
   setLength( pos );
}

auto_ptr<UserIDKey>
IDKeyToRequestPacket::getIDKey() const {
   int pos = USER_REQUEST_HEADER_SIZE;
   // Read type and size, and removeIDkey ID
   pos += 6 + 4;
   return auto_ptr<UserIDKey> ( new UserIDKey( this, pos ) );
}

uint32
IDKeyToRequestPacket::getRemoveIDkeyID() const {
   int pos = USER_REQUEST_HEADER_SIZE;
   return readLong( pos );
}

IDKeyToReplyPacket::IDKeyToReplyPacket( const IDKeyToRequestPacket* req,
                                        uint32 ownerUIN )
      : UserReplyPacket( USER_REPLY_HEADER_SIZE + 4,
                         PACKETTYPE_ID_KEY_TO_REPLY,
                         req )
{
   int pos = USER_REPLY_HEADER_SIZE;
   incWriteLong( pos, ownerUIN );
   setLength( pos );
}

uint32
IDKeyToReplyPacket::getOwnerUIN() const {
   return readLong( USER_REPLY_HEADER_SIZE );
}

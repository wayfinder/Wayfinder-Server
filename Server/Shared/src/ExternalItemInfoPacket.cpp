/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"

#include "ExternalItemInfoPacket.h"

#include "UserRightsMapInfo.h"
#include "SearchMatch.h"
#include "LangTypes.h"

ExternalItemInfoRequestPacket::
ExternalItemInfoRequestPacket( const SearchMatch& match,
                               const LangType& langType,
                               const UserUser* user )
      : RequestPacket( 65536,
                       DEFAULT_PACKET_PRIO,
                       Packet::PACKETTYPE_EXTERNALITEMINFO_REQUEST,
                       Packet::NO_PACKET_ID, // Packet ID
                       Packet::MAX_REQUEST_ID, 
                       Packet::MAX_MAP_ID )
{
   setResourceID( match.getExtSource() );
   int pos = REQUEST_HEADER_SIZE;
   UserRightsMapInfo rights(  MAX_UINT32, user );
   rights.save( this, pos );
   incWriteString( pos, match.getExtID() );
   incWriteLong ( pos, langType );
   incWriteLong( pos, match.getCoords().lat );
   incWriteLong( pos, match.getCoords().lon );
   incWriteString( pos, match.getName() );
   setLength( pos );
}

void
ExternalItemInfoRequestPacket::get( uint32& service,
                                    MC2String& extID,
                                    LangType& langType,
                                    UserRightsMapInfo& rights,
                                    MC2Coordinate& coord,
                                    MC2String& name ) const
{
   service = getResourceID();
   
   int pos = REQUEST_HEADER_SIZE;
   rights.load( this, pos );
   incReadString( pos, extID );
   langType = LangType::language_t( incReadLong( pos ) );
   coord.lat = incReadLong( pos );
   coord.lon = incReadLong( pos );
   incReadString( pos, name );
}

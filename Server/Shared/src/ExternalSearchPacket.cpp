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

#include "ExternalSearchPacket.h"
#include "SearchRequestParameters.h"
#include "UserRightsMapInfo.h"
#include "SearchReplyData.h"
#include "ExternalSearchRequestData.h"

ExternalSearchRequestPacket::
ExternalSearchRequestPacket( const ExternalSearchRequestData& data,
                             const UserUser* user )
   : RequestPacket( 65536, // packet size
                    DEFAULT_PACKET_PRIO,
                    Packet::PACKETTYPE_EXTERNALSEARCH_REQUEST,
                    Packet::NO_PACKET_ID, // Packet ID
                    Packet::MAX_REQUEST_ID,
                    Packet::MAX_MAP_ID )
{
   setResourceID( data.getService() );
   int pos = REQUEST_HEADER_SIZE;
   // Using empty maprights for now since we are not using it
   // in ExternalServiceModule yet.
   UserRightsMapInfo rights(  MAX_UINT32, user, MapRights() );   
   rights.save( this, pos );
   data.save( this, pos );
   setLength( pos );
}

void
ExternalSearchRequestPacket::get( ExternalSearchRequestData& data,
                                  UserRightsMapInfo& rights ) const
{
   int pos = REQUEST_HEADER_SIZE;
   rights.load( this, pos );
   data.load( this, pos );
}

ExternalSearchReplyPacket::
ExternalSearchReplyPacket( const RequestPacket* req,
                           const SearchReplyData& res )
      : ReplyPacket( 65536, // FIXME: Calculate this somehow.
                     Packet::PACKETTYPE_EXTERNALSEARCH_REPLY,
                     req,
                     StringTable::OK )
{
   int pos = REPLY_HEADER_SIZE;
   res.save( this, pos );
   incWriteLong( pos, 0 ); // top hits
   setLength( pos );
}

void
ExternalSearchReplyPacket::get( SearchReplyData& res, 
                                uint32& topHits ) const
{
   int pos = REPLY_HEADER_SIZE;
   res.load( this, pos );
   topHits = incReadLong( pos );
}

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
#include <memory>
#include "SearchTypes.h"
#include "Packet.h"
#include "SearchRequestPacket.h"
#include "SearchReplyPacket.h"

/////////////////////////////////////////////////////////////////////////
// reply
/////////////////////////////////////////////////////////////////////////

SearchReplyPacket::SearchReplyPacket( uint16 subType,
                                      const RequestPacket* p,
                                      int nbrPacks)
      : ReplyPacket( MAX_PACKET_SIZE * 3 * nbrPacks,
                     subType, p, 0 )
{
   mc2dbg4 << "SearchReplyPacket data" << endl
           << "IP " << getOriginIP() << endl << "Port " << getOriginPort()
           << "packetID " << getPacketID() << "RequestID " << getRequestID()
           << endl;
   setLength( REPLY_HEADER_SIZE );
   setStatus(StringTable::OK);
}


VanillaSearchReplyPacket::
VanillaSearchReplyPacket(const RequestPacket* p,
                         int nbrPacks )
      : SearchReplyPacket( Packet::PACKETTYPE_VANILLASEARCHREPLY, p,
                           nbrPacks )
{
   mc2dbg4 << "VanillaSearchReplyPacket Constructor start" << endl ;
   int i = REPLY_HEADER_SIZE;
   setLocationEmptyHack( false );
   quickIncWriteLong( i, 0 ); // initial number of added items
   setSizeData( 0 );
   setLength( REPLY_HEADER_SIZE + 8 +4 );
   mc2dbg4 << "VanillaSearchReplyPacket Constructor done" << endl ;
}


void VanillaSearchReplyPacket::getAllMatches(vector<VanillaMatch*>& matches)
{
   int position;
   int nbrMatches;
   VanillaMatch *vm = getFirstMatch( position,
                                     nbrMatches );
   matches.reserve(nbrMatches);
   
   while ( vm != NULL ) {
      matches.push_back(vm);
      vm = getNextMatch(position);
   }
   mc2dbg4 << "[VSRP]: getAllMatches added " << matches.size()
           << " matches - packet said " << nbrMatches << endl;
}

void
VanillaSearchReplyPacket::setLocationEmptyHack( bool empty )
{
   writeLong( REPLY_HEADER_SIZE + 8, empty );
}

bool
VanillaSearchReplyPacket::getLocationEmptyHack() const
{
   return readLong( REPLY_HEADER_SIZE + 8 ) != 0;
}


VanillaMatch *
VanillaSearchReplyPacket::getFirstMatch(
   int &position,
   int &nbrMatches ) const
{
   position = REPLY_HEADER_SIZE;
   nbrMatches = incReadLong( position );
   mc2dbg4 << "VanillaSearchReplyPacket::getFirstMatch nbrMatches " 
           << nbrMatches << endl;
   int dataSize;
   dataSize = incReadLong( position );
   // FIXME: locationEmptyHack not used
   bool locationEmptyHack;
   locationEmptyHack = incReadLong( position ) != 0;
   
   return getNextMatch( position );
}

VanillaMatch *
VanillaSearchReplyPacket::getNextMatch( int &position ) const
{
   mc2dbg8 << "[VSRP]: getNextMatch" << endl;
   if ( position < (int) getLength() ) {
      return static_cast<VanillaMatch*>
         (SearchMatch::createMatch(this, position));
   } else {
      return NULL;
   }
}

SearchMatchLink* 
VanillaSearchReplyPacket::getMatchesAsSearchLinks( int& nbrMatches ) const
{
   int position;
   VanillaMatch *vm = getFirstMatch( position,
                                     nbrMatches );
   SearchMatchLink* last = NULL;
   SearchMatchLink* res = NULL;
   SearchMatchLink* current = NULL;
   
   if ( vm != NULL ) {
      res = new SearchMatchLink( vm );
      last = res;
      vm = getNextMatch( position );
   }
   while( vm != NULL ) {
      current = new SearchMatchLink( vm );
      last->setNext( current );
      last = current;
      vm = getNextMatch( position );
   }

   return res;
}

// ========================================================================
//                                                 SearchMatchReplyPacket =

SearchMatchReplyPacket::SearchMatchReplyPacket( uint16 subType,
                                                const RequestPacket *p )
      : SearchReplyPacket( subType, p )
{
   init();
}

void
SearchMatchReplyPacket::init()
{
   mc2dbg4 << "SearchMatchReplyPacket Constructor start" << endl;
   int i = REPLY_HEADER_SIZE;
   quickIncWriteLong( i, 0 ); // initial number of added items
   updateSize( SEARCH_REPLY_HEADER_SIZE ); // length of packet
   mc2dbg4 << "SearchMatchReplyPacket Constructor done" << endl;
}


SearchMatch *
SearchMatchReplyPacket::getFirstMatch(int &position,
                                      int &nbrMatches ) const
{
   position = REPLY_HEADER_SIZE;
   nbrMatches = incReadLong( position );
   mc2dbg4 << "nbrMatches " << nbrMatches << endl;
   int dataSize;
   dataSize = incReadLong( position ); // inc the position
   switch (getSubType() ) {
      case Packet::PACKETTYPE_OVERVIEWSEARCHREPLYPACKET : {
         uint32 totalNbrMasks;
         // read past the total number of masks.
         totalNbrMasks = incReadLong( position );
         uint32 uof;
         uof = incReadLong( position );
         return static_cast<const OverviewSearchReplyPacket*>( this )
            ->getNextMatch( position );
      }
      break;
      default :
         mc2dbg1 << "SearchMatchReplyPacket::getFirstMatch "
                 << "unknown packetType: " << (int) getSubType() << endl;
         return NULL;
   };
}


SearchMatch*
SearchMatchReplyPacket::getNextMatch( int &position ) const {
   switch (getSubType() ) {
      case Packet::PACKETTYPE_OVERVIEWSEARCHREPLYPACKET :
         return static_cast<const OverviewSearchReplyPacket*>( this )
            ->getNextMatch( position );
         break;
      default :
         mc2dbg1 << "SearchMatchReplyPacket::getFirstMatch "
                 << "unknown packetType: " << (int) getSubType() << endl;
         return NULL;
   };
}


uint32
OverviewSearchReplyPacket::getTotalNbrMasks() const
{
   return readLong( SEARCH_REPLY_HEADER_SIZE );
}

void
OverviewSearchReplyPacket::setTotalNbrMasks( uint32 totNbrMasks )
{
   writeLong( SEARCH_REPLY_HEADER_SIZE, totNbrMasks );
}   

bool
OverviewSearchReplyPacket::getFull() const
{
   return readShort( SEARCH_REPLY_HEADER_SIZE+4 );
}

void
OverviewSearchReplyPacket::setFull( bool full )
{
   writeShort( SEARCH_REPLY_HEADER_SIZE+4, full );
}

uint32
OverviewSearchReplyPacket::getUniqueOrFull() const
{
   return readShort( SEARCH_REPLY_HEADER_SIZE + 4 + 2 );
}

void
OverviewSearchReplyPacket::setUniqueOrFull( uint32 uof )
{
   writeShort( SEARCH_REPLY_HEADER_SIZE + 4 + 2 , uof );
}


SearchMatch *
OverviewSearchReplyPacket::getNextMatch( int &position ) const
{
   OverviewMatch *result = NULL;
   if (position <
       (int) getSizeData())
   {
      // We know that it is OverviewMatch or very wrong.
      result =
         static_cast<OverviewMatch*>(SearchMatch::createMatch(this, position));
      result->setFromUniqueOrFullMatchPacket ( this->getUniqueOrFull() );
      result->setFromFullMatchPacket( this->getFull() );
   }
   return result;
}


void
OverviewSearchReplyPacket::addOverviewMatch( OverviewMatch *om )
{
   mc2dbg4 << "addOverviewMatch start" << endl;
   int position;
   position = getSizeData();
//   position = 4; // OVERVIEW_SEARCH_REPLY_HEADER_SIZE;
   // add size of header and first two uint32s.
   
   if (position < PACKET_FULL_LIMIT) {
      incNumberOfMatches();

      om->save(this, position);
      
   } // if      
   updateSize( position );
   mc2dbg4 << "addOverviewMatch end" << endl;
}

OverviewSearchReplyPacket::
OverviewSearchReplyPacket(const RequestPacket* req,
                          const vector<OverviewMatch*>& result)
      : SearchMatchReplyPacket(
         Packet::PACKETTYPE_OVERVIEWSEARCHREPLYPACKET, req)
{
   // Set stuff as in other constructor.
   setFull(false);
   setTotalNbrMasks( 0 );
   setStatus( StringTable::OK );
   updateSize( OVERVIEW_SEARCH_REPLY_HEADER_SIZE );
   // Add the matches.
   for( vector<OverviewMatch*>::const_iterator it = result.begin();
        it != result.end();
        ++it ) {
      if ( (*it) != NULL ) {
         addOverviewMatch(*it);
      }
   }
}



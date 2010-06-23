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
#include "Properties.h"

#include "TopRegionRequest.h"
#include "TopRegionPacket.h"

#include <algorithm>

TopRegionRequest::TopRegionRequest( uint32 reqID ) 
      : RequestWithStatus( reqID ),
        m_state( TOP_REGION )
{
   init();
}

TopRegionRequest::TopRegionRequest( Request* parent )
      : RequestWithStatus( parent ),
        m_state( TOP_REGION )
{
   setParentRequest(parent);
   init();
}

TopRegionRequest::~TopRegionRequest() {
   // Delete answer
   for ( TopRegionMatchesVector::iterator it = m_matches.begin() ;
         it != m_matches.end() ; ++it )
   {
      delete *it;
   }
}


void
TopRegionRequest::init()
{
   m_isCacheable = false; // assume we will not succeed completely
   // The Top Regions Request Packet
   uint32 mapSet = Properties::getUint32Property("MAP_SET_COUNT", 
                                                      MAX_UINT32);
   if (mapSet != MAX_UINT32)
      mapSet--;
   // If MAP_SET_COUNT is set (!= MAX_UINT32) one req packet should be sent
   // to each map module leader.
   // Should probably change the timeout/resends??
   m_repliesToReceive = 0;
   while (mapSet != (MAX_UINT32 - 1)) {
      m_repliesToReceive++;
      m_packetsReadyToSend.add( new PacketContainer( 
         new TopRegionRequestPacket( getID(), getNextPacketID() ),
         0, 0, MODULE_TYPE_MAP, PacketContainer::defaultResendTimeoutTime,
         PacketContainer::defaultResends, mapSet) );
      mc2dbg2 << "[TopRegReq] Sent req packet for mapSet: " << mapSet 
              << endl;
      if (mapSet == 0 || mapSet == MAX_UINT32)
         mapSet = MAX_UINT32 - 1;
      else
         mapSet--;
   }
}

namespace {
   /// Sorts topregion matches by id.
   class Sorter {
   public:
      bool operator()(const TopRegionMatch* a,
                      const TopRegionMatch* b) {
         return a->getID() < b->getID();
      }
   };
};

void 
TopRegionRequest::processPacket( PacketContainer* pack ) {
   switch( m_state ) {
   // stay in TOP_REGION state until all module sets have answered!
      case TOP_REGION : {
         TopRegionReplyPacket* p = static_cast<TopRegionReplyPacket*>(
            pack->getPacket() );
         m_repliesToReceive--;
         p->getTopRegionsAndIDTree( m_matches, m_wholeTree );

         // FIXME: should use getNbrOutstandingPackets() instead of 
         // m_repliesToReceive
         if (m_repliesToReceive <= 0) {
            m_state = DONE;
            setDone( true );
            // Sort the matches in a constistent way, which is
            // needed so that the TileMaps will be the same every time.
            std::sort( m_matches.begin(), m_matches.end(), Sorter() );
            // this result is cacheable
            m_isCacheable = true;
         }

      } break;

      case DONE :
         mc2log << error << "TopRegionRequest::processPacket received "
                   "packet in DONE state." << endl;
         break;
         
      case ERROR :
         mc2log << error << "TopRegionRequest::processPacket received "
                   "packet in ERROR state." << endl;
         
         break;
   }

   // Remove packet
   delete pack;
}


uint32 
TopRegionRequest::getNbrTopRegions() const {
   return m_matches.size();
}


const TopRegionMatch* 
TopRegionRequest::getTopRegion( uint32 i ) const {
   if ( i < m_matches.size() ) {
      return m_matches[ i ];
   } else {
      return NULL;
   }
}


const TopRegionMatch* 
TopRegionRequest::getTopRegionWithID( uint32 id ) const {
   // FIXME Less linear search
   for ( uint32 i = 0 ; i < m_matches.size() ; ++i ) {
      if ( m_matches[ i ]->getID() == id ) {
         // Found!
         return m_matches[ i ];
         break;
      }
   }
   return NULL;
}

const TopRegionMatch*
TopRegionRequest::getTopRegionWithBBoxOverlap(const MC2BoundingBox& bbox ) const
{
   for (uint32 i = 0; i < getNbrTopRegions(); i++) {
      const TopRegionMatch* reg = getTopRegion(i);
      const MC2BoundingBox regBbox = reg->getBoundingBox();
      if (regBbox.overlaps(bbox)) { 
         return reg;
      }
   }
   return NULL;
}

const TopRegionMatch*
TopRegionRequest::getTopRegionByCoordinate( const MC2Coordinate& coord ) const
{
   for (uint32 i = 0; i < getNbrTopRegions(); i++) {
      const TopRegionMatch* reg = getTopRegion(i);
      const MC2BoundingBox regBbox = reg->getBoundingBox();
      if (regBbox.contains(coord)) { 
         return reg;
      }
   }
   return NULL;
}

const TopRegionMatch*
TopRegionRequest::getTopRegionByName( const char* name ) const
{
   for ( TopRegionMatchesVector::const_iterator it = m_matches.begin();
         it != m_matches.end();
         ++it ) {
      const char* val = (*it)->getNames()->
         getBestName( LangTypes::english)->getName();
      if ( val == NULL ) {
         continue;
      }
      if ( StringUtility::strcasecmp( name, val ) == 0 ) {
         return *it;
      }
   }
   return NULL;
}
   

StringTable::stringCode 
TopRegionRequest::getStatus() const {
// report DONE
   switch ( m_state ) {
      case TOP_REGION:
         // to be able to somewhat gracefully handle map updates ie updates
         // of one of the available module sets we don't want to report timeout
         // if at least some of the reply packets have been received
         if (m_matches.size() == 0)
            return StringTable::TIMEOUT_ERROR;
         else {
            if (!isCacheable()) {
               mc2log << warn << "[TopRegionRequest] not complete, missing "
                      << "answer from at least one leader. Result is not "
                      << "cacheable." << endl;
            }
            return StringTable::OK;
         }
      case DONE:
         return StringTable::OK;
      case ERROR:
         return StringTable::INTERNAL_SERVER_ERROR;
   }

   // Unreachable code
   return StringTable::INTERNAL_SERVER_ERROR;
}


const ItemIDTree& 
TopRegionRequest::getWholeItemIDTree() const {
   return m_wholeTree;
}

const TopRegionMatchesVector&
TopRegionRequest::getTopRegionVector() const
{
   return m_matches;
}

const TopRegionMatch*
TopRegionRequest::getCountryForMapID( uint32 mapID ) const
{
   for ( TopRegionMatchesVector::const_iterator it = m_matches.begin() ;
         it != m_matches.end() ;
         ++it ) {
      const TopRegionMatch* curMatch = *it;
      if ( curMatch->getType() != TopRegionMatch::country ) {
         // Not interesting
         continue;
      }
      const ItemIDTree& idTree(curMatch->getItemIDTree());
      if ( idTree.containsMap( mapID ) ) {
         return curMatch;
      }
   }
   return NULL;
}

void
TopRegionRequest::addTopRegion( TopRegionMatch* match ) {
   m_matches.push_back( match );
}

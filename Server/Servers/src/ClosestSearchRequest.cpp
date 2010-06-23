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


#include "ClosestSearchRequest.h"
#include "CoordinateRequest.h"
#include "TopRegionRequest.h"
#include "SearchReplyPacket.h"
#include "SearchPacket.h"
#include "SearchSorting.h"

#include "MatchInfoPacket.h"
#include "UserRightsMapInfo.h"

#include "MapBits.h"
#include "DeleteHelpers.h"

#include <sstream>

#define PRINT_STATE_CHANGES

inline const char*
ClosestSearchRequest::stateAsString(ClosestSearchRequest::state_t inState)
{
   switch ( inState ) {
      case FINDING_COUNTRY:
         return "FINDING_COUNTRY";
      case TOP_REGION:
         return "TOP_REGION";
      case SEARCHING:
         return "SEARCHING";
      case ERROR:
         return "ERROR";
      case DONE_OK:
         return "DONE_OK";
   }
   return "UNKNOWN";
}

inline void
ClosestSearchRequest::setState(ClosestSearchRequest::state_t newState,
                               int line)
{
#ifdef PRINT_STATE_CHANGES
   if ( newState != m_state ) {
      if ( line < 0 ) {
         mc2dbg << "[CSR]: " 
                << stateAsString(m_state) << "->"
                << stateAsString(newState) << endl;
      } else {
         mc2dbg << "[CSR]: " << __FILE__ << ":" << line 
                << ' ' <<  stateAsString(m_state) << "->"
                << stateAsString(newState) << endl;
      }
   } // Don't print if the state doesn't change.
#endif
   m_state = newState;
}

// To be able to change this into printing the state
// always use SETSTATE instead of m_state=
#define SETSTATE(s) setState(s, __LINE__)
                    
                    
                    

void
ClosestSearchRequest::init( const SearchRequestParameters& params,
                            const MC2Coordinate& sortOrigin,
                            const MC2String& itemQuery)
{
   // If there is a timeout we cannot set the status.
   m_status = StringTable::TIMEOUT_ERROR;
   m_matchInfoPacketsWereSent = false;
   m_nbrSearchPacketsLeft = 0;
   m_params = params;
   m_sortOrigin = sortOrigin;
   m_searchString = itemQuery;
   
   if ( m_sortOrigin.isValid() ) {
      // Assume that the user wants the stuff distance sorted.
      m_params.setSortingType(SearchTypes::DistanceSort);
      SETSTATE(FINDING_COUNTRY);
      // Create the coordinate object. Language is not important
      m_coordReq = new CoordinateRequest(this, m_topReq);
      static const byte itemType = ItemTypes::streetSegmentItem;
      m_coordReq->addCoordinate(m_sortOrigin.lat, m_sortOrigin.lon,
                                COORDINATE_PACKET_RESULT_IDS,
                                1, &itemType);
      
      if ( enqueuePacketsFromRequest(m_coordReq) == 0 ) {
         SETSTATE(ERROR);
         m_status = StringTable::NOTOK;
      }
   } else {
      // Probably using top region.
      m_coordReq = NULL;
   }
}

ClosestSearchRequest::
ClosestSearchRequest( const RequestData& parentOrID,
                      const SearchRequestParameters& params,
                      const MC2Coordinate& sortOrigin,
                      const MC2String& itemQuery,
                      const TopRegionRequest* topReq,
                      const set<uint32>& allowedMaps)
      : SearchResultRequest(parentOrID), m_allowedMaps(allowedMaps)
{
   m_state = SEARCHING;
   m_topRegionReq = NULL;
   m_topReq = topReq;
   init(params, sortOrigin, itemQuery);
}

ClosestSearchRequest::
ClosestSearchRequest( const RequestData& parentOrID,
                      const SearchRequestParameters& params,
                      uint32 topRegionID,
                      const MC2String& itemQuery,
                      const TopRegionRequest* topReq,
                      const set<uint32>& allowedMaps )
   : SearchResultRequest(parentOrID), m_allowedMaps(allowedMaps)
{
   m_state = SEARCHING;
   m_topRegionReq = NULL;
   m_topReq = topReq;
   init( params, MC2Coordinate(), itemQuery );

   // FIXME: Much of this is copied from handleTopRegionReceived
   //        Common parts should be merged
   // Get the maps for the top region
   const TopRegionMatch* topRegion = topReq->getTopRegionWithID( topRegionID );
   if ( topRegion == NULL ) {
      SETSTATE( ERROR );
      m_status = StringTable::NOTOK;
      return;
   }
   // We now have our region.
   // Get the id:s
   const ItemIDTree& tree(topRegion->getItemIDTree());
   set<uint32> mapIDs;
   tree.getLowestLevelMapIDs(mapIDs);
   // Remove overview maps.
   for ( set<uint32>::iterator it = mapIDs.begin();
         it != mapIDs.end(); ) {
      if ( MapBits::isOverviewMap( *it ) ) {
         mapIDs.erase(it++);
      } else {
         ++it;
      }
   }
   mc2dbg << "[CRP]: Using country "
          << MC2CITE(topRegion->getName(LangTypes::english)) << endl;
   if ( mapIDs.empty() ) {
      // Is this ok? Shouldn't happen
      SETSTATE(DONE_OK);
   } else {
      // Found the right region. Create the packets and
      // change state. All in createSearchPackets.
      createSearchPackets(mapIDs);
      return;
   }
}

ClosestSearchRequest::~ClosestSearchRequest()
{
   delete m_coordReq;
   delete m_topRegionReq;
   STLUtility::deleteValues( m_matches );
   STLUtility::deleteValues( m_recvPackets );
}

bool
ClosestSearchRequest::mapAllowed(uint32 mapID) const
{
   if ( m_allowedMaps.empty() ) {
      return true;
   }
   if ( m_allowedMaps.find(mapID) != m_allowedMaps.end() ) {
      return true;
   } else {
      return false;
   }
}

StringTable::stringCode
ClosestSearchRequest::getStatus() const
{   
   switch ( m_state ) {
      case DONE_OK:
         return StringTable::OK;
      case ERROR:
         mc2dbg << "[CSR]: Returning status "
                << int(m_status) << endl;
         if ( m_status != StringTable::OK) {
            return m_status;
         } else {
            return StringTable::NOTOK;
         }
      default:
         return StringTable::TIMEOUT_ERROR;
   }
      
}

bool 
ClosestSearchRequest::requestDone()
{
   return (m_state == DONE_OK) ||
          (m_state == ERROR );
}

const vector<VanillaMatch*>&
ClosestSearchRequest::getMatches() const
{
   return m_matches;
}

inline void
ClosestSearchRequest::handleCountryFound()
{
   m_topRegionReq = new TopRegionRequest(this);
   if ( enqueuePacketsFromRequest(m_topRegionReq) ) {
      SETSTATE(TOP_REGION);
   } else {
      SETSTATE(ERROR);
   }
}

inline void
ClosestSearchRequest::createSearchPackets(set<uint32>& maps)
{
   vector<MC2BoundingBox> allowedBBoxes;
   vector<IDPair_t> allowedRegions;
   stringstream dbgStr;
   dbgStr << "[CSR]: Will search in maps {";
   for(set<uint32>::const_iterator it = maps.begin();
       it != maps.end();
       ++it ) {
      if ( mapAllowed( *it ) ) {
         if ( MapBits::isUnderviewMap(*it) ) {
            dbgStr << *it << ",";
         } else {
            dbgStr << hex << "0x" << *it << dec << ",";
         }
         SearchRequestPacket* reqPack =
            new SearchRequestPacket(*it,
                                    getNextPacketID(),
                                    getID(),
                                    SearchRequestPacket::USER_SEARCH,
                                    m_searchString.c_str(),
                                    m_params,
                                    allowedRegions,
                                    allowedBBoxes,
                                    m_sortOrigin,
                                    UserRightsMapInfo( *it,
                                                       getUser(),
                                                       ~MapRights() ) );
         
         enqueuePacket(reqPack, MODULE_TYPE_SEARCH);
         m_nbrSearchPacketsLeft++;
      } else {
         mc2dbg << "[CSR]: Map " << *it << " is not allowed" << endl;
      }
   }
   dbgStr << "}" << endl << ends;
   mc2dbg << dbgStr.str();

   if ( m_nbrSearchPacketsLeft == 0 ) {
      mc2log << info << "[CSR]: No maps to search in, that are allowed." 
             << endl;
      m_status = StringTable::OUTSIDE_ALLOWED_AREA;
      SETSTATE( ERROR );
      setDone( true );
   } else {
      SETSTATE( SEARCHING );
   }
}

inline void
ClosestSearchRequest::handleTopRegionReceived()
{
   const TopRegionMatch* topRegion = NULL;
   uint32 mapID = MAX_UINT32;

   CoordinateReplyPacket* crp = m_coordReq->getCoordinateReply();
   if ( crp != NULL ) {
      mapID = crp->getMapID();
      topRegion = m_topRegionReq->getCountryForMapID( mapID );
   }
   
   if ( topRegion != NULL) {
      // We now have our region.
      // Get the id:s
      const ItemIDTree& tree(topRegion->getItemIDTree());
      set<uint32> mapIDs;
      tree.getLowestLevelMapIDs(mapIDs);
      mc2dbg << "[CRP]: Using country "
             << MC2CITE(topRegion->getName(LangTypes::english)) << endl;
      if ( mapIDs.empty() ) {
         // Is this ok? Shouldn't happen
         SETSTATE(DONE_OK);
      } else {
         // Found the right region. Create the packets and
         // change state. All in createSearchPackets.
         createSearchPackets(mapIDs);
         return;
      }
   }
   
   // Strange number in coordinate reply.
   m_status = StringTable::NOTFOUND;
   SETSTATE(ERROR);
   mc2log << warn << "[CSR]: No country found for map "
          << hex << mapID << dec << endl;
}

inline int
ClosestSearchRequest::
sendMatchInfoPacket( const vector<VanillaMatch*>& matches)
{
   if ( matches.empty() ) {
      return 0;
   }

   MatchInfoRequestPacket* reqPack =
      new MatchInfoRequestPacket(matches,
                                 m_params.getRegionsInMatches(),
                                 m_params.getRequestedLang(),
                                 matches.front()->getMapID(),
                                 getNextPacketID(),
                                 getID() ) ;
   
   if ( reqPack->getNbrItems() != 0 ) {
      m_matchInfoPacketsWereSent = true;
      enqueuePacket(reqPack, MODULE_TYPE_MAP);      
      return 1; // One packet sent
   } else {
      // Generated no items
      delete reqPack;
   }
   
   return 0;   
}

inline void
ClosestSearchRequest::
handleSearchingVanillaReply( VanillaSearchReplyPacket* vrp)
{
   // Get the matches.
   vector<VanillaMatch*> packetMatches;
   vrp->getAllMatches(packetMatches);
   
   // There may be a need to send the matches to the MM
   // to fixup the house numbers.
   int nbrNewPacks = sendMatchInfoPacket(packetMatches);
   m_nbrSearchPacketsLeft += nbrNewPacks;
   // Put the matches in the result vector.
   SearchSorting::mergeSortedMatches(m_matches, packetMatches,
                                     m_params.getSortingType(),
                                     -1);
}


inline void
ClosestSearchRequest::
handleSearchingMatchInfoReply( MatchInfoReplyPacket* mirp)
{
   // The packet knows what to do.
   mirp->fixupMatches(m_matches);
}

inline void
ClosestSearchRequest::handleDoneOK()
{
   if ( m_matchInfoPacketsWereSent ) {
      // Update distances and re-sort the matches.
      mc2dbg << "[CSR]: Re-sorting may be needed, but not implemented"
             << endl;
   }
}

void
ClosestSearchRequest::processPacket( PacketContainer *pack ) 
{
   // When this function is called with NULL it means that
   // processSubRequestPacket should be run and enqueue stuff.
   switch ( m_state ) {
      case FINDING_COUNTRY:
         if ( processSubRequestPacket(m_coordReq,
                                      pack,
                                      m_status) ) {
            // Other request done
            if ( m_status == StringTable::OK ) {
               handleCountryFound();
            } else {
               SETSTATE(ERROR);
            }
         } else {
            // Keep on keepin' on
         }
         break;
      case TOP_REGION:
         if ( processSubRequestPacket(m_topRegionReq,
                                      pack,
                                      m_status) ) {
            // Other request done
            if ( m_status == StringTable::OK ) {
               handleTopRegionReceived();
            } else {
               SETSTATE(ERROR);
            }
         } else {
            // Keep on keepin' on
         }
         break;
      case SEARCHING: {
         // Pack shouldn't be NULL in this state
         Packet* thePacket = pack->getPacket();         
         --m_nbrSearchPacketsLeft;
         switch ( thePacket->getSubType() ) {
            case Packet::PACKETTYPE_VANILLASEARCHREPLY: {
               VanillaSearchReplyPacket* vrp =
                  static_cast<VanillaSearchReplyPacket*>(pack->getPacket());
               handleSearchingVanillaReply(vrp);
            }
            break;
            case Packet::PACKETTYPE_MATCHINFOREPLY: {
               MatchInfoReplyPacket* mirp =
                  static_cast<MatchInfoReplyPacket*>(thePacket);
               handleSearchingMatchInfoReply(mirp);               
            }
            break;
                              
            default:
               break;
         }
         // Save so that it can be deleted later.
         m_recvPackets.push_back( pack );
         if ( m_nbrSearchPacketsLeft == 0 ) {
            // FIXME: Should expand vanillaregionid:s too?
            SETSTATE(DONE_OK);
         }
         break;
      }
      default:
      break;
   }
}
